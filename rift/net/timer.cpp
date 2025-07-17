#include <sys/timerfd.h>
#include <string.h>

#include "rift/net/timer.h"
#include "rift/common/log.h"
#include "rift/common/util.h"

namespace rift {
    Timer::Timer() : FdEvent(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)) {
        DEBUGLOG("timer fd = %d", m_fd);
        // 把fd的可读事件放到eventloop中监听
        listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer, this));
    }

    Timer::~Timer() {

    }

    void Timer::onTimer() {
        // 处理缓冲区数据，防止下一次继续触发可读事件
        char buf[8];
        while (1) {
            if ((read(m_fd, buf, 8) == -1) && (errno == EAGAIN)) {
                break;
            }
        }

        // 执行定时任务
        int64_t now = getNowMs();

        std::vector<TimeEvent::s_ptr> tmps;
        std::vector<std::pair<int64_t, std::function<void()>>> tasks;

        ScopeMutex<Mutex> lock(m_mutex);
        auto it = m_pending_events.begin();
        
        for (it = m_pending_events.begin(); it != m_pending_events.end(); ++it) {
            if (it->first <= now) {
                if (!it->second->isCanceled()) {
                    tmps.push_back(it->second);
                    tasks.push_back(std::make_pair(it->second->getArriveTime(), it->second->getCallback()));
                }
            }
            else {
                break;
            }
        }

        m_pending_events.erase(m_pending_events.begin(), it);
        lock.unlock();

        // 把重复的event重新添加到定时器中
        for (auto it = tmps.begin(); it != tmps.end(); ++it) {
            if ((*it)->isRepeated()) {
                (*it)->resetArriveTime();
                addTimeEvent(*it);
            }
        }

        resetArriveTime();

        for (auto& i: tasks){
            if (i.second) {
                i.second();
            }
        }
    }

    void Timer::resetArriveTime() {
        ScopeMutex<Mutex> lock(m_mutex);
        auto tmp = m_pending_events;
        lock.unlock();

        if (tmp.empty()) {
            return;
        }

        int64_t now = getNowMs();

        auto it = tmp.begin();
        int64_t time_diff = 0;
        if (it->second->getArriveTime() > now) {
            time_diff = it->second->getArriveTime() - now;
        }
        else{
            time_diff = 100;
        }

        timespec ts;
        memset(&ts, 0, sizeof(ts));
        ts.tv_sec = time_diff / 1000;
        ts.tv_nsec = (time_diff % 1000) * 1000000;
        
        itimerspec newValue;
        memset(&newValue, 0, sizeof(newValue));
        newValue.it_value = ts;

        int rt = timerfd_settime(m_fd, 0, &newValue, nullptr);
        if (rt != 0) {
            ERRORLOG("timerfd_settime error, errno = %d, error = %s", errno, strerror(errno));
        }
        DEBUGLOG("timer reset to time_diff = %lld", now + time_diff);
    }

    void Timer::addTimeEvent(TimeEvent::s_ptr event) {
        bool reset_timerfd = false;

        ScopeMutex<Mutex> lock(m_mutex);
        if (m_pending_events.empty()) {
            reset_timerfd = true;
        }
        else {
            auto it = m_pending_events.begin();
            if (it->second->getArriveTime() > event->getArriveTime()) {
                reset_timerfd = true;
            }
        }
        m_pending_events.emplace(event->getArriveTime(), event);
        lock.unlock();

        if (reset_timerfd) {
            resetArriveTime();
        }
    }

    void Timer::deleteTimeEvent(TimeEvent::s_ptr event) {
        event->setCanceled(true);

        ScopeMutex<Mutex> lock(m_mutex);
        auto begin = m_pending_events.lower_bound(event->getArriveTime());
        auto end = m_pending_events.upper_bound(event->getArriveTime());
        auto it = begin;
        for (it = begin; it != end; ++it) {
            if (it->second == event) {
                m_pending_events.erase(it);
                break;
            }
        }
        lock.unlock();
        
        DEBUGLOG("success delete timer event at arrive time %lld", event->getArriveTime());
    }
}