#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

// 静态成员需要在类外初始化,且不需要再加static
MprpcConfig MprpcApplicaiton::m_config;

void ShowArgsHelp()
{
    std::cout << "format: command -i <configfile>" << std::endl;
}

// 类外实现静态函数,不用再带static标识
void MprpcApplicaiton::Init(int argc, char **argv)
{
    if (argc < 2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 开始加载配置文件
    m_config.LoadConfigFile(config_file.c_str());

    // std::cout << "rpcserverip:" << m_config.Load("rpcserverip") << std::endl;
    // std::cout << "rpcserverport:" << m_config.Load("rpcserverport") << std::endl;
    // std::cout << "zookeeperip:" << m_config.Load("zookeeperip") << std::endl;
    // std::cout << "zookeeperport:" << m_config.Load("zookeeperport") << std::endl;
}

// 获取单例的唯一接口
MprpcApplicaiton &MprpcApplicaiton::GetInstance()
{
    static MprpcApplicaiton app;
    return app;
}

MprpcConfig &MprpcApplicaiton::GetConfig()
{
    return m_config;
}