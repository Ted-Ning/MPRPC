#include "mprpcchannel.h"
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"
#include <string>
#include <rpcheader.pb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

/*
    在框架内部,RpcRrovider和RpcConsumer协商好之间通信用的protobuf数据类型
    service_name method_name args 定义proto的message类型,进行数据的序列化和反序列化
    header_size(四个字节) + header_str + args_str

    message RpcHeader
    {
        bytes service_name = 1;
        bytes method_name = 2;
        uint32 args_size =3;
    }
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
    // 获取服务列表
    const google::protobuf::ServiceDescriptor *sd = method->service();
    // 获取服务名
    std::string service_name = sd->name();
    // 获取方法名
    std::string method_name = method->name();

    // 获取参数的序列化字符串长度 args_size
    uint32_t args_size = 0;
    std::string args_str;
    // 对请求参数做序列化并保存到args_str中
    if (request->SerializeToString(&args_str))
    {
        // 获取序列化后的参数长度,保存到args.size中
        args_size = args_str.size();
    }
    else
    {
        // std::cout << " serialize request error! " << std::endl;
        controller->SetFailed(" serialize request error! ");
        return;
    }

    // 定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    // 对请求头做序列化并保存到rpc_header_str中
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        // 将序列化后的请求头长度保存到header_size中
        header_size = rpc_header_str.size();
    }
    else
    {
        // std::cout << " serialize rpc header error! " << std::endl;
        controller->SetFailed(" serialize rpc header error! ");
        return;
    }

    // 组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    // header_size,固定占前四个字节
    send_rpc_str.insert(0, std::string((char *)&header_size, 4));
    // rpcheader,序列化后的请求头
    send_rpc_str += rpc_header_str;
    // args_str,序列化后的参数
    send_rpc_str += args_str;

    // 打印调试信息
    // std::cout << "================================================" << std::endl;
    // std::cout << " header_size: " << header_size << std::endl;
    // std::cout << " rpc_header_str: " << rpc_header_str << std::endl;
    // std::cout << " service_name: " << service_name << std::endl;
    // std::cout << " method_name: " << method_name << std::endl;
    // std::cout << " args_str: " << args_str << std::endl;
    // std::cout << "================================================" << std::endl;

    // 使用tcp编程,完成rpc方法的远程调用,客户端一般对高并发没要求
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        // std::cout << " create socket error! errno: " << errno << std::endl;
        // std::string errorinfo = " create socket error! errno: ";
        // errorinfo += errno;

        char errtxt[512] = {0};
        sprintf(errtxt, " create socket error! errno:%d ", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // GetInstance通过单例模式返回一个mprapplication对象
    // 该对象具有一个静态mprpcconfig对象m_config,调用m_config的Load函数加载配置项参数
    // std::string ip = MprpcApplicaiton::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplicaiton::GetInstance().GetConfig().Load("rpcserverport").c_str());

    // 调用方从原来的从配置文件读取ip端口号,改为从zk注册服务中心查询ip和端口号
    ZKClient zkCli;
    zkCli.Start();
    std::string method_path = "/" + service_name + "/" + method_name;
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist! ");
        return;
    }
    int idx = host_data.find(':');
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid! ");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc服务节点
    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        char errtxt[512] = {0};
        sprintf(errtxt, " connect error! errno:%d ", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    // 发送rpc请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        // std::cout << " send error! errno: " << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, " send error! errno:%d ", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    // 接受rpc请求的响应
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        // std::cout << " recv error! errno: " << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, " recv error! errno:%d ", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    // 反序列化响应结果
    // 该写法存在bug,未反序列化的数据中存在/0,转存到string类型的response_str中时遇到/0会直接结束
    // std::string response_str(recv_buf, 0, recv_size);
    // if (!response->ParseFromString(response_str))
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        // std::cout << "parse error! response_str: " << recv_buf << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, " parse error! response_str:%s ", recv_buf);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    close(clientfd);
}