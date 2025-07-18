#include <assert.h>

#include "rift/net/io_thread.h"
#include "rift/common/log.h"
#include "rift/common/util.h"

namespace rift {
    IOThread::IOThread() {
        int rt = sem_init(&m_init_semaphore, 0, 0);
        assert(rt == 0);

        rt = sem_init(&m_start_semaphore, 0, 0);
        assert(rt == 0);

        pthread_create(&m_thread, nullptr, &IOThread::Main, this);
        sem_wait(&m_init_semaphore);

        DEBUGLOG("IOThread [%d] created", m_thread_id);
    }

    IOThread::~IOThread() {
        m_event_loop->terminate();
        pthread_join(m_thread, nullptr);
        sem_destroy(&m_init_semaphore);
        sem_destroy(&m_start_semaphore);

        if (m_event_loop) {
            delete m_event_loop;
            m_event_loop = nullptr;
        }
    }

    void* IOThread::Main(void* arg) {
        IOThread* thread = static_cast<IOThread*>(arg);
        thread->m_event_loop = new Eventloop();
        thread->m_thread_id = getThreadId();

        sem_post(&thread->m_init_semaphore);
        DEBUGLOG("IOThread [%d] created, wait start semaphore", thread->m_thread_id);

        sem_wait(&thread->m_start_semaphore);
        DEBUGLOG("IOThread [%d] start loop", thread->m_thread_id);

        thread->m_event_loop->loop();
        DEBUGLOG("IOThread [%d] stop loop", thread->m_thread_id);
        
        return nullptr;
    }

    Eventloop* IOThread::getEventloop() {
        return m_event_loop;
    }

    void IOThread::start() {
        DEBUGLOG("Now invoke IOThread %d", m_thread_id);
        sem_post(&m_start_semaphore);
    }

    void IOThread::join() {
        pthread_join(m_thread, nullptr);
    }
}