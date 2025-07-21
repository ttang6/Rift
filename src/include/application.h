#ifndef RIFT_INCLUDE_APPLICATION_H
#define RIFT_INCLUDE_APPLICATION_H

#include<mutex>

#include "config.h"
#include "channel.h" 
#include "controller.h"

//rpc基础类，负责框架的一些初始化操作
class Application
{
    public:
    static void init(int argc,char **argv);
    static Application & getInstance();
    static void deleteInstance();
    static Config& getConfig();
    private:
    static Config m_config;
    static Application * m_application; //全局唯一单例访问对象
    static std::mutex m_mutex;
    Application(){}
    ~Application(){}
    Application(const Application&)=delete;
    Application(Application&&)=delete;
};

#endif 