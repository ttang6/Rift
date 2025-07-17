#ifndef __RIFT_NET_TIMER_H__
#define __RIFT_NET_TIMER_H__

#include <map>
#include "rift/common/mutex.h"
#include "rift/net/time_event.h"
#include "rift/net/fd_event.h"

namespace rift {
    class Timer : public FdEvent {
        public:
            Timer();

            ~Timer();

            void addTimeEvent(TimeEvent::s_ptr event);

            void deleteTimeEvent(TimeEvent::s_ptr event);

            void onTimer();
        
        private:
            void resetArriveTime();

        private:
            std::multimap<int64_t, TimeEvent::s_ptr> m_pending_events;
            Mutex m_mutex;

    };
}
#endif