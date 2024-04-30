#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include "google/protobuf/service.h"
#include "google/protobuf/descriptor.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpConnection.h"

// 框架提供的专门服务腹部rpc服务的网络对象类
class RpcProvider
{
public:
    // 这里是框架提供给外部使用的,可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务节点,开始提提供rpc远程服务调用
    void Run();

private:
    // 组合EventLoop
    muduo::net::EventLoop m_eventloop;

    // 服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;
        // 保存服务方法
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap;
    };

    // 存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // 新的socket连接回调;
    // 使用muduo库要注意加muduo的名词空间作用域
    void OnConnection(const muduo::net::TcpConnectionPtr &);
    // 已建立连接用户的读写时间回调
    void OnMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);
    // Closure的回调操作,用于序列化rpc的相应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr &, google::protobuf::Message *);
};
