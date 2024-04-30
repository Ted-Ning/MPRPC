#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>
#include "logger.h"

// 全局watcher观察器 zkserver的zkclient的通知
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT) // 回调的消息类型和会话相关的消息类型
    {
        if (state == ZOO_CONNECTED_STATE) // zkclient和zkserver连接成功
        {
            // 获取附加给zh的信号量
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            // 连接成功给信号量资源+1
            sem_post(sem);
        }
    }
}

ZKClient::ZKClient()
{
}
ZKClient::~ZKClient()
{
    if (m_zhandle != nullptr)
    {
        // 关闭句柄,释放资源
        zookeeper_close(m_zhandle);
    }
}
// 客户端启动连接zkserver
void ZKClient::Start()
{
    std::string host = MprpcApplicaiton::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplicaiton::GetInstance().GetConfig().Load("zookeeperport");
    // zk初始化的固定格式ip:port
    std::string connstr = host + ":" + port;
    /*
        zookeeeper_mt:多线程版本
        zookeeper的API客户端程序提供了三个线程
        1.API调用线程
        2.网络I/O线程 pthread_create poll
        3.watcher回调线程
    */
    // session timeout 30s zkclient 网络I/O线程 1/3*timeout 时间发送ping消息作为心跳消息
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    // 多线程下异步通讯,nullptr一定连接错误,但有返回值不代表连接成功,需要watcher回调进一步操作
    if (nullptr == m_zhandle)
    {
        std::cout << " zookeeper_init error! " << std::endl;
        LOG_ERR(" zookeeper_init error! ");
        exit(EXIT_FAILURE);
    }

    // 初始化一个资源为0的信号量
    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    // 因为资源为0,信号量会再次阻塞,等待watcher的操作
    sem_wait(&sem);
    // 运行到此代表zk客户端与服务端连接成功
    std::cout << " zookeeper_init success! " << std::endl;
    LOG_INFO(" zookeeper_init success! ");
}
// 在zkserver上根据指定的path创建znode节点;
void ZKClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    // 先判断path表示的znode节点是否存在,如果存在,就不在重复创建了
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    // 表示path的znode节点不存在
    if (ZNONODE == flag)
    {
        // 创建指定path的znode节点
        flag = zoo_create(m_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        // ZOK代表操作成功
        if (flag == ZOK)
        {
            std::cout << " znode create success...path: " << path << std::endl;
            LOG_INFO(" znode create success...path:%s ", path);
        }
        else
        {
            std::cout << "flag: " << flag << std::endl;
            std::cout << " znode create erroe... path: " << path << std::endl;
            LOG_ERR(" flag:%d znode create erroe... path:%s ", flag, path);
            exit(EXIT_FAILURE);
        }
    }
}
// 根据参数指定的节点路径,获得znode节点的值
std::string ZKClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if (flag != ZOK)
    {
        std::cout << " get znode error... path: " << path << std::endl;
        LOG_ERR(" get znode error... path:%s ", path);
        return "";
    }
    else
    {
        return buffer;
    }
}