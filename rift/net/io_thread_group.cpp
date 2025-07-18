#include "rift/net/io_thread_group.h"
#include "rift/common/log.h"

namespace rift {
    IOThreadGroup::IOThreadGroup(int size) : m_size{size} {
        m_io_thread_groups.resize(size);
        for (int i = 0; i < size; ++i) {
            m_io_thread_groups[i] = new IOThread();
        }
    }

    IOThreadGroup::~IOThreadGroup() {
        
    }

    void IOThreadGroup::start() {
        for (int i = 0; i < m_size; ++i) {
            m_io_thread_groups[i]->start();
        }
    }

    void IOThreadGroup::join() {
        for (int i = 0; i < m_size; ++i) {
            m_io_thread_groups[i]->join();
        }
    }

    IOThread* IOThreadGroup::getIOThread() {
        if (m_index >= m_size || m_index == -1) {
            m_index = 0;
        }

        return m_io_thread_groups[m_index++];  // 返回++是为了轮询分配，每次调用都是下一个线程
    }
}
