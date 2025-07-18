#ifndef RIFT_NET_IO_THREAD_H
#define RIFT_NET_IO_THREAD_H

#include <pthread.h>
#include <semaphore.h>

#include "rift/net/eventloop.h"

namespace rift {
    class IOThread {
        public:
            IOThread();

            ~IOThread();

            Eventloop* getEventloop();

            void start();

            void join();
        
        public:
            static void* Main(void* arg);

        private:

            pid_t m_thread_id {0}; 
            pthread_t m_thread {0};

            Eventloop* m_event_loop {nullptr}; // 当前io线程的eventloop

            sem_t m_init_semaphore;
            sem_t m_start_semaphore;
    };
}

#endif