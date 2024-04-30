#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"

int main(int argc, char **argv)
{
    // 整个程序启动以后,像使用mprpc框架来享受rpc服务调用,一定需要先调用框架的初始化函数(只初始化一次)
    MprpcApplicaiton::Init(argc, argv);

    // 用rpc框架的Channel,初始化一个stub
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());

    // 调用方需要提供request和相关参数
    fixbug::GetFriendsListRequest request;
    request.set_userid(2001);

    // 声明response接收rpc方法的响应
    fixbug::GetFriendsListResponse response;

    // 声明rpccontroller
    MprpcController controller;

    // 调用方法
    stub.GetFriendsList(&controller, &request, &response, nullptr);

    // 调用完成后,读调用结果
    // 先通过controller判断是否顺利执行request请求和response响应
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (response.result().errcode() == 0)
        {
            std::cout << "rpc GetFriendsList response success! " << std::endl;
            /*通过查看GetFriendsListResponse类可以找打如下两个提供好的函数,可用于访问返回的好友列表
                int friends_size() const;
                const std::string& friends(int index) const;
            */
            int size = response.friends_size();
            for (int i = 0; i < size; ++i)
            {
                std::cout << " index: " << (i + 1) << " name: " << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendsList response error: "
                      << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}