#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

// 这里是框架提供给外部使用的,可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    // 创建ServiceInfo结构体 用于存储某种服务的方法信息
    ServiceInfo service_info;
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象service的方法数量
    int methodCnt = pserviceDesc->method_count();

    std::cout << "service_name: " << service_name << std::endl;
    LOG_INFO("service_name:%s", service_name.c_str());

    for (int i = 0; i < methodCnt; ++i)
    {
        // 获取了服务对象指定下标的服务方法的描述(抽象描述)
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        // 将该服务的方法名字和指针存入结构体
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        std::cout << "method_name: " << method_name << std::endl;
        LOG_INFO("method_name:%s", method_name.c_str());
    }
    // 存储服务的名字
    service_info.m_service = service;
    // 将服务的名字和方法信息存入类的服务表中
    m_serviceMap.insert({service_name, service_info});
}

// 启动rpc服务节点,开始提提供rpc远程服务调用
// run方法封装了网络代码,将网络代码和业务代码分离,尽可能将少的参数暴露给框架使用者
void RpcProvider::Run()
{
    // GetInstance通过单例模式返回一个mprapplication对象
    // 该对象具有一个静态mprpcconfig对象m_config,调用m_config的Load函数加载配置项参数
    std::string ip = MprpcApplicaiton::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplicaiton::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建tcpserver对象
    muduo::net::TcpServer server(&m_eventloop, address, "RpcProvider");

    // 绑定连接回调和消息读写回调方法
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1,
                                        std::placeholders::_2, std::placeholders::_3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面,让rpc client可以从zk上发现服务
    ZKClient zkCli;
    zkCli.Start();
    // service_name为永久性节点,method_name为临时性节点
    for (auto &sp : m_serviceMap)
    {
        //  /service_name
        std::string service_path = "/" + sp.first;
        // 服务节点,只传路径,没有数据,数据长度为0,节点类型缺省为永久节点
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            //  /service_name/method_name
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // 方法节点,路径为服务/方法,数据为ip地址:端口号,长度为数据长度,节点类型选择临时节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;
    LOG_INFO("RpcProvider start service at ip:%s port:%hu", ip.c_str(), port);

    // 启动网络服务
    server.start();
    m_eventloop.loop();
}

// 新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 如果和rpc client断开
        conn->shutdown();
    }
}

/*
    在框架内部,RpcRrovider和RpcConsumer协商好之间通信用的protobuf数据类型
    service_name method_name args 定义proto的message类型,进行数据的序列化和反序列化
    header_size(四个字节) + header_str + args_str
*/

// 已建立连接用户的读写时间,如果远程有一个rpc服务的请求,那么OnMessage方法就会相应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp)
{
    // 网络上接受的远程rpc调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取前四个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    // 根据header_size读取数据投的原始字符流,反序列化数据,得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;

    std::string service_name;
    std::string method_name;
    uint32_t args_size;

    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 数据头反序列化失败
        std::cout << " rpc_header_str: " << rpc_header_str << " parse error " << std::endl;
        LOG_ERR("rpc_header_str:%s parse error! ", rpc_header_str);
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    // std::cout << "================================================" << std::endl;
    // std::cout << " header_size: " << header_size << std::endl;
    // std::cout << " rpc_header_str: " << rpc_header_str << std::endl;
    // std::cout << " service_name: " << service_name << std::endl;
    // std::cout << " method_name: " << method_name << std::endl;
    // std::cout << " args_str: " << args_str << std::endl;
    // std::cout << "================================================" << std::endl;

    // 获取service对象和method对象
    // 在使用map的时候一般不要用[]的方式寻找key-value,因为当该key不存在时会创建新的键值对,改用find
    // 先判断服务是否存在
    auto sit = m_serviceMap.find(service_name);
    if (sit == m_serviceMap.end())
    {
        std::cout << service_name << " is not exist! " << std::endl;
        return;
    }
    // 在判断服务中是否存在对应的方法
    auto mit = sit->second.m_methodMap.find(method_name);
    if (mit == sit->second.m_methodMap.end())
    {
        std::cout << service_name << " : " << method_name << " is not exist! " << std::endl;
        return;
    }
    // 获取对应服务的对应方法
    google::protobuf::Service *service = sit->second.m_service;
    const google::protobuf::MethodDescriptor *method = mit->second;

    // 生成rpc方法调用的请求request和响应的response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << "requst parse error, content:" << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用,绑定一个Closure的回调函数
    /*
    template <typename Class, typename Arg1, typename Arg2>
    inline Closure* NewCallback(Class* object, void (Class::*method)(Arg1, Arg2),
                            Arg1 arg1, Arg2 arg2)
    */
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this,
                                                                                                 &RpcProvider::SendRpcResponse,
                                                                                                 conn, response);

    // 在框架上根据远端rpc请求,调用当前rpc节点上发布的方法
    // 框架不应调用具体对象具体方法,例如:new UserService().Login(controller,request,response,done)
    // 通过protobuf提供的方法实现
    service->CallMethod(method, nullptr, request, response, done);
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn,
                                  google::protobuf::Message *response)
{
    std::string response_str;
    // 对response进行序列化,保存到response_str
    // SerializePartialToString
    if (response->SerializeToString(&response_str))
    {
        // 序列化成功后,通过网络把响应结果返回给调用端
        conn->send(response_str);
    }
    else
    {
        // 序列化错误
        std::cout << "serialize response_str error" << std::endl;
    }
    // 模拟http短连接服务,由rpcprovider主动断开连接
    conn->shutdown();
}