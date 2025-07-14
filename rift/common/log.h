#ifndef RIFT_COMMON_LOG_H
#define RIFT_COMMON_LOG_H

#include <cstring>
#include <string>
#include <queue>
#include <memory>

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

    #define DEBUGLOG(str, ...) do {\
        rift::LogEvent* event = new rift::LogEvent(rift::LogLevel::DEBUG);\
        ::std::string msg = event->toString() + rift::formatString(str, ##__VA_ARGS__);\
        delete event;\
        msg += "\n";\
        rift::Logger::getGlobalLogger()->pushLog(msg);\
        rift::Logger::getGlobalLogger()->log();\
    } while(0)
    
    enum class LogLevel{
        UNKNOWN,
        DEBUG,
        INFO,
        ERROR,
    };

    class Logger{
        public:
            typedef std::shared_ptr<Logger> s_ptr;

            void pushLog(const std::string& msg);
            void log();

        public:
            static Logger* getGlobalLogger();


        private:
            LogLevel m_set_level;
            std::queue<std::string> m_buffer;
    };

    std::string LogLevelToString(LogLevel level);

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