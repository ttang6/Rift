#ifndef __RIFT_WAKEUP_FD_EVENT_H__
#define __RIFT_WAKEUP_FD_EVENT_H__

#include "net/fd_event.h"

namespace rift {
    class WakeupFdEvent : public FdEvent {
        public:
            WakeupFdEvent(int fd);

            ~WakeupFdEvent();


            void wakeup();

        private:

    };
}

#endif