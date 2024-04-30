#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

// 封装的ZK客户端类
class ZKClient
{
public:
    ZKClient();
    ~ZKClient();
    // 客户端启动连接zkserver
    void Start();
    // 在zkserver上根据指定的path创建znode节点;
    void Create(const char *path, const char *data, int datalen, int state = 0);
    // 根据参数指定的节点路径,获得znode节点的值
    std::string GetData(const char *path);

private:
    // zk的客户端句柄
    zhandle_t *m_zhandle;
};