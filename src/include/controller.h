#ifndef RIFT_INCLUDE_CONTROLLER_H
#define RIFT_INCLUDE_CONTROLLER_H

#include<google/protobuf/service.h>
#include<string>
//用于描述RPC调用的控制器
//其主要作用是跟踪RPC方法调用的状态、错误信息并提供控制功能(如取消调用)。
class Controller : public google::protobuf::RpcController
{
    public:
        Controller();
        void Reset();
        bool Failed() const;
        std::string ErrorText() const;
        void SetFailed(const std::string &reason);

        //目前未实现具体的功能
        void StartCancel();
        bool IsCanceled() const;
        void NotifyOnCancel(google::protobuf::Closure* callback);
    private:
        bool m_failed;//RPC方法执行过程中的状态
        std::string m_errText;//RPC方法执行过程中的错误信息
};

#endif