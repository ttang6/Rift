#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <string.h>
#include "rift/net/eventloop.h"
#include "rift/common/log.h"
#include "rift/common/util.h"

#define ADD_TO_EPOLL(event) \
    auto it = m_listen_fds.find(event->getFd()); \
    int op = EPOLL_CTL_ADD; \
    if (it != m_listen_fds.end()) { \
        op = EPOLL_CTL_MOD; \
    } \
    epoll_event tmp = event->getEpollEvent(); \
    INFOLOG("epoll event type [%d]", (int)tmp.events); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if (rt == -1) { \
        ERRORLOG("failed epoll_ctl when add fd, errno [%d], error info [%s]", errno, strerror(errno)); \
    } \
    DEBUGLOG("add fd [%d] to epoll", event->getFd());

#define DELETE_FROM_EPOLL(event) \
    auto it = m_listen_fds.find(event->getFd()); \
    if (it == m_listen_fds.end()) { \
        return; \
    } \
    int op = EPOLL_CTL_DEL; \
    epoll_event tmp = event->getEpollEvent(); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if (rt == -1) { \
        ERRORLOG("failed epoll_ctl when delete fd, errno [%d], error info [%s]", errno, strerror(errno)); \
    } \
    DEBUGLOG("delete fd [%d] from epoll", event->getFd());

namespace rift {
    static thread_local Eventloop* t_cur_eventloop = nullptr;
    static int g_epoll_max_timeout = 10000;
    static int g_epoll_events_num = 1024;
    
    Eventloop::Eventloop() {
        if (t_cur_eventloop) {
            ERRORLOG("fail to create event loop, this thread already has a event loop");
            exit(0);
        }

        m_thread_id = getThreadId();
        m_epoll_fd = epoll_create(1024);

        if (m_epoll_fd < 0) {
            ERRORLOG("fail to create event loop, epoll_create failed, error info [%d]", errno);
            exit(0);
        }

        initWakeupFdEvent();
        initTimer();

        INFOLOG("event loop created in thread [%d]", m_thread_id);
        t_cur_eventloop = this;


    }

    Eventloop::~Eventloop() {
        close(m_epoll_fd);
        if (m_wakeup_fd_event) {
            delete m_wakeup_fd_event;
            m_wakeup_fd_event = nullptr;
        }
        if (m_timer) {
            delete m_timer;
            m_timer = nullptr;
        }
    }

    void Eventloop::initTimer() {
        m_timer = new Timer();
        addEpollEvent(m_timer);
    }

    void Eventloop::addTimeEvent(TimeEvent::s_ptr event) {
        if (m_timer) {
            m_timer->addTimeEvent(event);
        }
    }
    
    void Eventloop::initWakeupFdEvent() {
        m_wakeup_fd = eventfd(0, EFD_NONBLOCK); // 非阻塞
        if (m_wakeup_fd < 0) {
            ERRORLOG("fail to create event loop, eventfd failed, error info [%d]", errno);
            exit(0);
        }

        m_wakeup_fd_event = new WakeupFdEvent(m_wakeup_fd);  
        m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this]() {
            char buf[8];
            while(read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN) {
                continue;
            }
            DEBUGLOG("read full bytes from wakeup, fd [%d]", m_wakeup_fd);
        });

        addEpollEvent(m_wakeup_fd_event);
    }

    void Eventloop::loop() {
        while (!m_stop_flag) {
            ScopeMutex<Mutex> lock(m_mutex);
            std::queue<std::function<void()>> tmp_tasks;
            m_pending_tasks.swap(tmp_tasks);
            lock.unlock();

            while (!tmp_tasks.empty()) {
                std::function<void()> task = tmp_tasks.front();
                tmp_tasks.pop();
                if (task) {
                    task();
                }
            }

            int timeout = g_epoll_max_timeout;
            epoll_event result_events[g_epoll_events_num];
            DEBUGLOG("epoll_wait begin");
            int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_events_num, timeout);
            DEBUGLOG("epoll_wait end on rt [%d]", rt);

            if (rt < 0) {
                ERRORLOG("epoll_wait error, errno = ", errno);
            } 
            else {
                for (int i = 0; i < rt; ++i) {
                    epoll_event trigger_event = result_events[i];
                    FdEvent* fd_event = static_cast<FdEvent*>(trigger_event.data.ptr);

                    if (fd_event == nullptr) {
                        continue;
                    }
                    if (trigger_event.events & EPOLLIN) {
                        DEBUGLOG("fd [%d] trigger EPOLLIN event", fd_event->getFd());
                        addPendingTask(fd_event->handler(FdEvent::IN_EVENT));
                    }
                    if (trigger_event.events & EPOLLOUT) {
                        DEBUGLOG("fd [%d], trigger EPOLLOUT event", fd_event->getFd());
                        addPendingTask(fd_event->handler(FdEvent::OUT_EVENT));
                    }
                }
            }
        }
    }


    void Eventloop::wakeup() {
        m_wakeup_fd_event->wakeup();
    }

    void Eventloop::terminate(){
        m_stop_flag = true;
    }

    void Eventloop::handleWakeup() {

    }

    void Eventloop::addEpollEvent(FdEvent* event) {
        if (isInLoopThread()) {
            ADD_TO_EPOLL(event);
        }
        else {
            auto cb = [this, event]() {
                ADD_TO_EPOLL(event);
            };
            addPendingTask(cb, true);
        }
    }

    void Eventloop::deleteEpollEvent(FdEvent* event){
        if (isInLoopThread()) {
            DELETE_FROM_EPOLL(event);
        }
        else {
            auto cb = [this, event]() {
                DELETE_FROM_EPOLL(event);
            };
            addPendingTask(cb, true);
        }
    }

    void Eventloop::addPendingTask(std::function<void()> task, bool is_wakeup) {
        ScopeMutex<Mutex> lock(m_mutex);
        m_pending_tasks.push(task);
        lock.unlock();

        if (is_wakeup) {
            wakeup();
        }
    }

    bool Eventloop::isInLoopThread() {
        return getThreadId() == m_thread_id;
    }

}
