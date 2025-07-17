#ifndef RIFT_NET_EVENTLOOP_H
#define RIFT_NET_EVENTLOOP_H

#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include "rift/common/mutex.h"
#include "rift/net/fd_event.h"
#include "rift/net/wakeup_fd.h"
#include "rift/net/timer.h"
#include "rift/net/time_event.h"

namespace rift {
    class Eventloop {
        public:
            Eventloop();

            ~Eventloop();

            void loop();

            void wakeup();

            void terminate();

            void addEpollEvent(FdEvent* event);

            void deleteEpollEvent(FdEvent* event);

            bool isInLoopThread(); // 执行当前这个函数的线程是不是eventloop的线程

            void addPendingTask(std::function<void()> task, bool is_wakeup = false);

            void addTimeEvent(TimeEvent::s_ptr event);
        
        private:
            void handleWakeup();

            void initWakeupFdEvent();

            void initTimer();


        private:
            pid_t m_thread_id {0};

            int m_epoll_fd = 0;

            WakeupFdEvent* m_wakeup_fd_event;

            int m_wakeup_fd = 0; //确保成员变量声明

            bool m_stop_flag {false};

            std::set<int> m_listen_fds;

            std::queue<std::function<void()>> m_pending_tasks;

            Mutex m_mutex;

            Timer* m_timer {nullptr};
    };
}

#endif