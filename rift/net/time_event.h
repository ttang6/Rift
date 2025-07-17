#ifndef __RIFT_NET_TIME_EVENT_H__
#define __RIFT_NET_TIME_EVENT_H__

#include <functional>
#include <memory>

namespace rift {
    class TimeEvent {
        public:
            typedef std::shared_ptr<TimeEvent> s_ptr;

            TimeEvent(int interval, bool is_repeated, std::function<void()> cb);

            int64_t getArriveTime() const {
                return m_arrive_time;
            }

            void setCanceled(bool value) {
                m_is_canceled = value;
            }

            bool isCanceled() const {
                return m_is_canceled;
            }

            bool isRepeated() const {
                return m_is_repeated;
            }

            std::function<void()> getCallback() const {
                return m_task;
            }

            void resetArriveTime();

        private:
            int64_t m_arrive_time;
            int64_t m_interval;
            bool m_is_repeated {false};
            bool m_is_canceled {false};

            std::function<void()> m_task;
    };
}

#endif