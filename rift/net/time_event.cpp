#include "rift/net/time_event.h"
#include "rift/common/log.h"
#include "rift/common/util.h"

namespace rift {
    TimeEvent::TimeEvent(int interval, bool is_repeated, std::function<void()> cb)
        : m_interval(interval), m_is_repeated(is_repeated), m_task(cb) {
            resetArriveTime();
    }

    void TimeEvent::resetArriveTime() {
        m_arrive_time = getNowMs() + m_interval;
        DEBUGLOG("success reset arrive time, will execute at [%lld]", m_arrive_time);
    }
}