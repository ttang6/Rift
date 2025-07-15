#ifndef RIFT_COMMON_LOG_H
#define RIFT_COMMON_LOG_H

#include <cstring>
#include <string>
#include <queue>
#include <memory>
#include "rift/common/config.h"
#include "rift/common/mutex.h"

namespace rift{
    template<typename... Args> // Args接受任意数量和类型的参数， T接受一个固定类型的参数
    // 这个函数类似C语言的sprintf，但返回一个std::string而不是写入缓冲区，用于将可变参数按照指定格式转换为字符串
    std::string formatString(const char* str, Args&&... args){ // Args&& 是万能引用，如果Args参数是左值，则使用左值引用，如果Args参数是右值，则使用右值引用
        int size = snprintf(nullptr, 0, str, ::std::forward<Args>(args)...); // 计算需要多少空间来存储格式化后的字符串，nullptr和0表示只计算长度而不实际写入
        // std::forward 用于实现完美转发，如果Args参数是左值，则使用左值引用，如果Args参数是右值，则使用右值引用
        std::string result;
        if (size > 0){  // 如果长度有效，则进行真的写入
            result.resize(size); 
            snprintf(&result[0], size + 1, str, ::std::forward<Args>(args)...); // size + 1表示包含字符串的终止符
        }

        return result;
    }

    /*
    #define DEBUGLOG(str, ...) do {\
        rift::LogEvent* event = new rift::LogEvent(rift::LogLevel::DEBUG);\
        ::std::string msg = event->toString() + rift::formatString(str, ##__VA_ARGS__);\
        delete event;\
        msg += "\n";\
        rift::Logger::getGlobalLogger()->pushLog(msg);\
        rift::Logger::getGlobalLogger()->log();\
    } while(0)
    */

    #define DEBUGLOG(str, ...) \
        do { \
            if (rift::Logger::getGlobalLogger()->getLogLevel() <= rift::LogLevel::DEBUG) { \
                std::string debug_msg = (new rift::LogEvent(rift::LogLevel::DEBUG))->toString() + " [" + std::string(__FILE__) + ":" + \
                std::to_string(__LINE__) + "]\t" + rift::formatString(str, ##__VA_ARGS__); \
                debug_msg += "\n"; \
                rift::Logger::getGlobalLogger()->pushLog(debug_msg); \
                rift::Logger::getGlobalLogger()->log(); \
            } \
        } while(0)
    
    #define INFOLOG(str, ...)\
        do {\
            if (rift::Logger::getGlobalLogger()->getLogLevel() <= rift::LogLevel::INFO) { \
                std::string info_msg = (new rift::LogEvent(rift::LogLevel::INFO))->toString() + " [" + std::string(__FILE__) + ":" + \
                std::to_string(__LINE__) + "]\t" + rift::formatString(str, ##__VA_ARGS__);\
                info_msg += "\n";\
                rift::Logger::getGlobalLogger()->pushLog(info_msg);\
                rift::Logger::getGlobalLogger()->log();\
            }\
        } while(0)\

    #define ERRORLOG(str, ...)\
        do {\
            if (rift::Logger::getGlobalLogger()->getLogLevel() <= rift::LogLevel::ERROR) { \
                std::string error_msg = (new rift::LogEvent(rift::LogLevel::ERROR))->toString() + " [" + std::string(__FILE__) + ":" + \
                std::to_string(__LINE__) + "]\t" + rift::formatString(str, ##__VA_ARGS__);\
                error_msg += "\n";\
                rift::Logger::getGlobalLogger()->pushLog(error_msg);\
                rift::Logger::getGlobalLogger()->log();\
            }\
        } while(0)
    
    enum LogLevel{
        UNKNOWN,
        DEBUG,
        INFO,
        ERROR,
    };

    class Logger{
        public:
            typedef std::shared_ptr<Logger> s_ptr;

            Logger(LogLevel level) : m_set_level(level) {}

            void pushLog(const std::string& msg); // 将日志消息添加到缓冲区
            void log(); // 将缓冲区的日志消息打印到控制台

            LogLevel getLogLevel() const{
                return m_set_level;
            }

        public:
            static Logger* getGlobalLogger();
            static void initGlobalLogger();


        private:
            LogLevel m_set_level;
            std::queue<std::string> m_buffer;

            Mutex m_mutex;
    };

    std::string LogLevelToString(LogLevel level);
    LogLevel StringToLogLevel(const std::string& log_level);

    class LogEvent{
        public:
            LogEvent(LogLevel level) : m_level(level){}

            std::string getFileName() const{ // const表示此函数不能修改成员变量
                return m_file_name;
            }

            LogLevel getLogLevel() const{
                return m_level;
            }

            std::string toString();

        private:
            std::string m_file_name; // file name
            int32_t m_file_line; // line number
            int32_t m_pid; // process id
            int32_t m_thread_id; // thread id

            LogLevel m_level; // log level
    };
}

#endif