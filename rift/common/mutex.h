#ifndef RIFT_COMMON_MUTEX_H
#define RIFT_COMMON_MUTEX_H

#include <pthread.h>

namespace rift{
    template <class T>
    class ScopeMutex{ // 将mutex封装成一个RAII对象，避免代码块因各种情况锁没有释放（比如抛出异常时会自动执行析构，但如果用的是unlock()则不会执行）
        public:
            ScopeMutex(T& mutex) : m_mutex(mutex) {
                m_mutex.lock();
                m_is_lock = true;
            }

            ~ScopeMutex(){
                m_mutex.unlock();
                m_is_lock = false;
            }

            void lock(){
                if (m_is_lock){
                    return;
                }
                m_mutex.lock();
                m_is_lock = true;
            }

            void unlock(){
                if (!m_is_lock){
                    return;
                }
                m_mutex.unlock();
                m_is_lock = false;
            }

        private:
            T& m_mutex;
            bool m_is_lock {false};

    };

    class Mutex{
        public:
            Mutex() {
                pthread_mutex_init(&m_mutex, nullptr);
            }

            ~Mutex() {
                pthread_mutex_destroy(&m_mutex);
            }

            void lock(){
                pthread_mutex_lock(&m_mutex);
            }

            void unlock(){
                pthread_mutex_unlock(&m_mutex);
            }

        private:
            pthread_mutex_t m_mutex;
    };
}

#endif