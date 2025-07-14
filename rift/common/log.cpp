#include <iostream>
#include <sstream>
#include <sys/time.h>
#include "rift/common/log.h"
#include "rift/common/util.h"
#include "rift/common/config.h"

namespace rift{
    static Logger* g_logger = nullptr;

    Logger* Logger::getGlobalLogger(){ // 如果在这里获取全局logger，可能会导致竟态问题
        return g_logger;
    }

    void Logger::initGlobalLogger(){ // 多线程启动前手动初始化全局logger
        LogLevel g_log_level = StringToLogLevel(Config::getGlobalConfig()->m_log_level);
        printf("Init log level: [%s]\n", LogLevelToString(g_log_level).c_str());
        g_logger = new Logger(g_log_level);
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

    LogLevel StringToLogLevel(const std::string& log_level){
        if (log_level == "DEBUG"){
            return LogLevel::DEBUG;
        } else if (log_level == "INFO"){
            return LogLevel::INFO;
        } else if (log_level == "ERROR"){
            return LogLevel::ERROR;
        } else {
            return LogLevel::UNKNOWN;
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
           << "[" << m_pid << ":" << m_thread_id << "]";
        
        return ss.str();
    }

    void Logger::pushLog(const std::string& msg){
        ScopeMutex<Mutex> lock(m_mutex);
        m_buffer.push(msg);
        lock.unlock();
    }
    // m_buffer作为全局单例logger的成员，不加锁会导致core dumped，因为pushlog和log可能被两个线程同时调用
    void Logger::log(){ 
        ScopeMutex<Mutex> lock(m_mutex);
        std::queue<std::string> tmp = m_buffer;
        m_buffer.swap(tmp);
        lock.unlock();

        while (!tmp.empty()){
            std::string msg = tmp.front();
            tmp.pop();
            std::cout << msg.c_str();
        }
    }
    
}