#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

// UserService原来是一个本地服务,提供两个进程内的本地方法,Login和Register
class UserService : public fixbug::UserServiceRpc // 使用在rpc服务发布端(rpc服务提供者)
{
public:
    // 本地的Login业务
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service:Login" << std::endl;
        std::cout << "name: " << name << " pwd: " << pwd << std::endl;

        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service:Register" << std::endl;
        std::cout << " id: " << id << " name: " << name << " pwd: " << pwd << std::endl;

        return true;
    }

    // 重写基类的Login函数
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done) override
    {
        // 框架给业务上报求情参数LoginRequest,应用获取相应的数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name, pwd);

        // 把响应写入Loginresponse
        fixbug::ResultCode *code = response->mutable_result();

        // 模拟业务成功
        code->set_errcode(0);
        code->set_errmsg("");

        // 模拟业务失败
        // code->set_errcode(1);
        // code->set_errmsg("Login error");

        response->set_sucess(login_result);

        // 执行回调操作,执行响应对象的序列化和网络发送
        done->Run();
    };

    // 重写基类注册服务
    virtual void Register(::google::protobuf::RpcController *controller,
                          const ::fixbug::RegisterRequest *request,
                          ::fixbug::RegisterResponse *response,
                          ::google::protobuf::Closure *done) override
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        int ret = Register(id, name, pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_sucess(ret);

        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 调用框架的初始化操作 例:provider -i congig.config
    MprpcApplicaiton::Init(argc, argv);

    // provider是一个rpc网络服务网对象,把UserServer对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService);

    // 启动一个rpc服务发布节点 Run以后,进程进入阻塞状态,等待远程rpc调用请求
    provider.Run();
    return 0;
}
