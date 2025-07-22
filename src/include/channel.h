#ifndef RIFT_INCLUDE_CHANNEL_H
#define RIFT_INCLUDE_CHANNEL_H
// 此类是继承自google::protobuf::RpcChannel
// 目的是为了给客户端进行方法调用的时候，统一接收的
#include <google/protobuf/service.h>

#include "zookeeperutil.h"

class Channel : public google::protobuf::RpcChannel
{
    public:
        Channel(bool connectNow);
        
        virtual ~Channel(){}

        void CallMethod(const ::google::protobuf::MethodDescriptor *method, // protobuf自带的method
                        ::google::protobuf::RpcController *controller,
                        const ::google::protobuf::Message *request,
                        ::google::protobuf::Message *response,
                        ::google::protobuf::Closure *done) override; // override可以验证是否是虚函数
                        
    private:
        int m_clientfd; // 存放客户端套接字
        std::string service_name;
        std::string m_ip;
        uint16_t m_port;
        std::string method_name;
        int m_idx; // 用来划分服务器ip和port的下标
        bool newConnect(const char *ip, uint16_t port);
        std::string QueryServiceHost(ZClient *zclient, std::string service_name, std::string method_name, int &idx);
};
#endif
