#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>
#include "logger.h"

// UserService原来是一个本地服务,提供两个进程内的本地方法,Login和GetFriendLists
class FriendService : public fixbug::FriendServiceRpc
{
private:
    // 模拟好友列表
    std::vector<std::string> vec = {"张三", "李四", "王五"};

public:
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::cout << " do GetFriendList service! "
                  << " userid: " << userid << std::endl;
        return vec;
    }

    // 重写基类方法
    void GetFriendsList(::google::protobuf::RpcController *controller,
                        const ::fixbug::GetFriendsListRequest *request,
                        ::fixbug::GetFriendsListResponse *response,
                        ::google::protobuf::Closure *done) override
    {
        uint32_t id = request->userid();
        std::vector<std::string> friendslist = GetFriendsList(id);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for (std::string &name : friendslist)
        {
            // add_friends会返回一个字符串指针
            std::string *p = response->add_friends();
            // 将指针指向的地址指定为frendslist的成员,实现向response的friendslist中添加朋友
            *p = name;
        }

        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 模拟普通日志
    LOG_INFO("first log message!");
    // 模拟错误日志
    LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    // 调用框架的初始化操作 例:provider -i congig.config
    MprpcApplicaiton::Init(argc, argv);

    // provider是一个rpc网络服务网对象,把FriendService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点 Run以后,进程进入阻塞状态,等待远程rpc调用请求
    provider.Run();
    return 0;
}
