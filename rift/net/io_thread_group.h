#ifndef RIFT_NET_IO_THREAD_GROUP_H
#define RIFT_NET_IO_THREAD_GROUP_H

#include <vector>

#include "rift/net/io_thread.h"
#include "rift/common/log.h"

namespace rift {
    class IOThreadGroup {
        public:
            IOThreadGroup(int size);

            ~IOThreadGroup();

            void start();

            void join();

            IOThread* getIOThread();

        private:
            std::vector<IOThread*> m_io_thread_groups;
            int m_index {0};
            int m_size {0};
    };
}

#endif