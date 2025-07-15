#ifndef RIFT_NET_EVENTLOOP_H
#define RIFT_NET_EVENTLOOP_H

#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include "rift/common/mutex.h"
#include "net/fd_event.h"
#include "net/wakeup_fd.h"

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
        
        private:
            void handleWakeup();

            void initWakeupFdEvent();


        private:
            pid_t m_thread_id {0};

            int m_epoll_fd = 0;

            WakeupFdEvent* m_wakeup_fd_event;

            int m_wakeup_fd = 0; //确保成员变量声明

            bool m_stop_flag {false};

            std::set<int> m_listen_fds;

            std::queue<std::function<void()>> m_pending_tasks;

            Mutex m_mutex;
    };
}

#endif