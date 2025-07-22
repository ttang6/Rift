#ifndef RIFT_INCLUDE_LOGGER_H
#define RIFT_INCLUDE_LOGGER_H

#include<glog/logging.h>
#include<string>
//采用RAII的思想
class Logger
{
    public:
        //构造函数，自动初始化glog
        explicit Logger(const char *argv0)
        {
            google::InitGoogleLogging(argv0);
            FLAGS_colorlogtostderr=true;//启用彩色日志
            FLAGS_logtostderr=true;//默认输出标准错误
        }
        ~Logger(){
            google::ShutdownGoogleLogging();
        }
        //提供静态日志方法
        static void Info(const std::string &message)
        {
            LOG(INFO)<<message;
        }
        static void Warning(const std::string &message){
            LOG(WARNING)<<message;
        }
        static void Error(const std::string &message){
            LOG(ERROR)<<message;
        }
        static void Fatal(const std::string& message) {
            LOG(FATAL) << message;
        }
    //禁用拷贝构造函数和重载赋值函数
    private:
        Logger(const Logger&)=delete;
        Logger& operator=(const Logger&)=delete;
};

#endif