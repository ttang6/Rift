#include <iostream>
#include <sstream>
#include <sys/time.h>
#include "rift/common/log.h"
#include "rift/common/util.h"

namespace rift{
    static Logger* g_logger = nullptr;

    Logger* Logger::getGlobalLogger(){
        if (g_logger == nullptr){
            g_logger = new Logger();
        }

        return g_logger;
    }
    
    std::string LogLevelToString(LogLevel level){
        switch (level){
            case LogLevel::DEBUG:
                return "DEBUG";

            case LogLevel::INFO:
                return "INFO";

            case LogLevel::ERROR:
                return "ERROR";

            default:
                return "UNKNOWN";
        }
    }

    std::string LogEvent::toString(){
        struct timeval now_time;
        gettimeofday(&now_time, nullptr);
        
        struct tm now_time_t;
        localtime_r(&now_time.tv_sec, &now_time_t);

        char buf[128];
        strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);
        std::string time_str(buf);

        int ms = now_time.tv_usec / 1000;
        time_str = time_str + "." + std::to_string(ms);

        m_pid = getPid();
        m_thread_id = getThreadId();

        std::stringstream ss;
        ss << "[" << LogLevelToString(m_level) << "] "
           << "[" << time_str << "] "
           << "[" << std::string(__FILE__) << ":" << __LINE__ << "] ";
        
        return ss.str();
    }

    void Logger::pushLog(const std::string& msg){
        m_buffer.push(msg);
    }

    void Logger::log(){
        while (!m_buffer.empty()){
            std::string msg = m_buffer.front();
            m_buffer.pop();

            std::cout << msg << std::endl;
        }
    }
    
}