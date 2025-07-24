#ifndef RIFT_INCLUDE_PROVIDER_H
#define RIFT_INCLUDE_PROVIDER_H

#include<functional>
#include<string>
#include<unordered_map>
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include<google/protobuf/descriptor.h>

#include "google/protobuf/service.h"
#include "zookeeperutil.h"

class Provider
{
    public:
        //这里是提供给外部使用的，可以发布rpc方法的函数接口。
        void notifyService(google::protobuf::Service* service);
        ~Provider();
        //启动rpc服务节点，开始提供rpc远程网络调用服务
        void run();

    private:
        muduo::net::EventLoop event_loop;
        struct serviceInfo
        {
            google::protobuf::Service* service;
            std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> method_map;
        };
        
        std::unordered_map<std::string, serviceInfo>service_map;//保存服务对象和rpc方法
        
        void onConnection(const muduo::net::TcpConnectionPtr& conn);
        void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp receive_time);
        void sendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response);
};

#endif 







