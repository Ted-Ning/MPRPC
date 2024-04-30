#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"

int main(int argc, char **argv)
{
    // 整个程序启动以后,像使用mprpc框架来享受rpc服务调用,一定需要先调用框架的初始化函数(只初始化一次)
    MprpcApplicaiton::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    /*
        在protobuf中有服务提供方XXXServiceRpc,就会有对应的XXXServiceRpc_Stub用于服务调用方
        该方法没有默认构造函数 需要一个Channel参数初始化
        UserServiceRpc_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
        当调用服务里的方法时,都通过调用Channel里的Callmethod函数实现
        void UserServiceRpc_Stub::Login(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                              const ::fixbug::LoginRequest* request,
                              ::fixbug::LoginResponse* response,
                              ::google::protobuf::Closure* done) {
        channel_->CallMethod(descriptor()->method(0),
                       controller, request, response, done);
        }

        rpc框架在调用方的代码实现,关键在于如何实现Channel,将方法调用的序列化/反序列化/网络发送统一交给Channel完成

    */
    // 用rpc框架的Channel,初始化一个stub
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    // 调用方需要提供request和相关参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    // 声明response接收rpc方法的响应
    fixbug::LoginResponse response;
    // 调用方法
    stub.Login(nullptr, &request, &response, nullptr);

    // 调用完成后,读调用结果
    if (response.result().errcode() == 0)
    {
        std::cout << "rpc login response success: " << response.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error: " << response.result().errmsg() << std::endl;
    }

    // 演示调用远程发布的rpc方法Register
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("li si");
    req.set_pwd("654321");
    // 声明response接收rpc方法的响应
    fixbug::RegisterResponse rsp;

    // 以同步方式发起rpc请求,等待返回结果
    stub.Register(nullptr, &req, &rsp, nullptr);

    // 调用完成后,读调用结果
    if (rsp.result().errcode() == 0)
    {
        std::cout << "rpc Register response success: " << rsp.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc Register response error: " << rsp.result().errmsg() << std::endl;
    }

    return 0;
}