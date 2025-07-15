#include <unistd.h>
#include "net/wakeup_fd.h"
#include "rift/common/log.h"

namespace rift {
    WakeupFdEvent::WakeupFdEvent(int fd) : FdEvent(fd) {}

    WakeupFdEvent::~WakeupFdEvent() {}
    
    void WakeupFdEvent::wakeup() {
        char buf[8] = {'a'};
        int rt = write(m_fd, buf, 8);

        if (rt == -1) {
            ERRORLOG("write to wakkup fd less than 8 bytes, fd [%d]", m_fd);
        }

    }

}