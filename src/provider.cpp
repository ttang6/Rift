#include "provider.h"
#include "application.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include <iostream>

// 注册服务对象及其方法，以便服务端能够处理客户端的RPC请求
void Provider::notifyService(google::protobuf::Service *service) {
    // 服务端需要知道客户端想要调用的服务对象和方法，
    // 这些信息会保存在一个数据结构（如 ServiceInfo）中。
    serviceInfo service_info;

    // 参数类型设置为 google::protobuf::Service，是因为所有由 protobuf 生成的服务类
    // 都继承自 google::protobuf::Service，这样我们可以通过基类指针指向子类对象，
    // 实现动态多态。

    // 通过动态多态调用 service->GetDescriptor()，
    // GetDescriptor() 方法会返回 protobuf 生成的服务类的描述信息（ServiceDescriptor）。
    const google::protobuf::ServiceDescriptor *psd = service->GetDescriptor();

    // 通过 ServiceDescriptor，我们可以获取该服务类中定义的方法列表，
    // 并进行相应的注册和管理。

    // 获取服务的名字
    std::string service_name = psd->name();
    // 获取服务端对象service的方法数量
    int method_count = psd->method_count();

    // 打印服务名
    std::cout << "service_name=" << service_name << std::endl;

    // 遍历服务中的所有方法，并注册到服务信息中
    for (int i = 0; i < method_count; ++i) {
        // 获取服务中的方法描述
        const google::protobuf::MethodDescriptor *pmd = psd->method(i);
        std::string method_name = pmd->name();
        std::cout << "method_name=" << method_name << std::endl;
        service_info.method_map.emplace(method_name, pmd);  // 将方法名和方法描述符存入map
    }
    service_info.service = service;  // 保存服务对象
    service_map.emplace(service_name, service_info);  // 将服务信息存入服务map
}

// 启动RPC服务节点，开始提供远程网络调用服务
void Provider::run() {
    // 读取配置文件中的RPC服务器IP和端口
    std::string ip = Application::getInstance().getConfig().load("rpcserverip");
    int port = atoi(Application::getInstance().getConfig().load("rpcserverport").c_str());

    // 使用muduo网络库，创建地址对象
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    std::shared_ptr<muduo::net::TcpServer> server = std::make_shared<muduo::net::TcpServer>(&event_loop, address, "Provider");

    // 绑定连接回调和消息回调，分离网络连接业务和消息处理业务
    server->setConnectionCallback(std::bind(&Provider::onConnection, this, std::placeholders::_1));
    server->setMessageCallback(std::bind(&Provider::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置muduo库的线程数量
    server->setThreadNum(4);

    // 将当前RPC节点上要发布的服务全部注册到ZooKeeper上，让RPC客户端可以在ZooKeeper上发现服务
    ZClient zkclient;
    zkclient.start();  // 连接ZooKeeper服务器
    // service_name为永久节点，method_name为临时节点
    for (auto &sp : service_map) {
        // service_name 在ZooKeeper中的目录是"/"+service_name
        std::string service_path = "/" + sp.first;
        zkclient.create(service_path.c_str(), nullptr, 0);  // 创建服务节点
        for (auto &mp : sp.second.method_map) {
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);  // 将IP和端口信息存入节点数据
            // ZOO_EPHEMERAL表示这个节点是临时节点，在客户端断开连接后，ZooKeeper会自动删除这个节点
            zkclient.create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // RPC服务端准备启动，打印信息
    std::cout << "Provider start service at ip:" << ip << " port:" << port << std::endl;

    // 启动网络服务
    server->start();
    event_loop.loop();  // 进入事件循环
}

// 连接回调函数，处理客户端连接事件
void Provider::onConnection(const muduo::net::TcpConnectionPtr &conn) {
    if (!conn->connected()) {
        // 如果连接关闭，则断开连接
        conn->shutdown();
    }
}

// 消息回调函数，处理客户端发送的RPC请求
void Provider::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp receive_time) {
    std::cout << "onMessage" << std::endl;

    // 从网络缓冲区中读取远程RPC调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();

    // 使用protobuf的CodedInputStream反序列化RPC请求
    google::protobuf::io::ArrayInputStream raw_input(recv_buf.data(), recv_buf.size());
    google::protobuf::io::CodedInputStream coded_input(&raw_input);

    uint32_t header_size{};
    coded_input.ReadVarint32(&header_size);  // 解析header_size

    // 根据header_size读取数据头的原始字符流，反序列化数据，得到RPC请求的详细信息
    std::string rpc_header_str;
    rift::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size{};

    // 设置读取限制
    google::protobuf::io::CodedInputStream::Limit msg_limit = coded_input.PushLimit(header_size);
    coded_input.ReadString(&rpc_header_str, header_size);
    // 恢复之前的限制，以便安全地继续读取其他数据
    coded_input.PopLimit(msg_limit);

    if (rpcHeader.ParseFromString(rpc_header_str)) {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    } else {
        Logger::Error("rpcHeader parse error");
        return;
    }

    std::string args_str;  // RPC参数
    // 直接读取args_size长度的字符串数据
    bool read_args_success = coded_input.ReadString(&args_str, args_size);
    if (!read_args_success) {
        Logger::Error("read args error");
        return;
    }

    // 获取service对象和method对象
    auto it = service_map.find(service_name);
    if (it == service_map.end()) {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }
    auto mit = it->second.method_map.find(method_name);
    if (mit == it->second.method_map.end()) {
        std::cout << service_name << "." << method_name << " is not exist!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.service;  // 获取服务对象
    const google::protobuf::MethodDescriptor *method = mit->second;  // 获取方法对象

    // 生成RPC方法调用请求的request和响应的response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();  // 动态创建请求对象
    if (!request->ParseFromString(args_str)) {
        std::cout << service_name << "." << method_name << " parse error!" << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();  // 动态创建响应对象

    // 绑定回调函数，用于在方法调用完成后发送响应
    google::protobuf::Closure *done = google::protobuf::NewCallback<Provider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this,
                                                                                                 &Provider::sendRpcResponse,
                                                                                                 conn, response);

    // 在框架上根据远端RPC请求，调用当前RPC节点上发布的方法
    service->CallMethod(method, nullptr, request, response, done);  // 调用服务方法
}

// 发送RPC响应给客户端
void Provider::sendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response) {
    std::string response_str;
    if (response->SerializeToString(&response_str)) {
        // 序列化成功，通过网络把RPC方法执行的结果返回给RPC调用方
        conn->send(response_str);
    } else {
        std::cout << "serialize error!" << std::endl;
    }
    // conn->shutdown(); // 模拟HTTP短链接，由RpcProvider主动断开连接
}

// 析构函数，退出事件循环
Provider::~Provider() {
    std::cout << "~Provider()" << std::endl;
    event_loop.quit();  // 退出事件循环
}