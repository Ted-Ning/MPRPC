#pragma once

#include <unordered_map>
#include <string>

// 框架读取配置文件类
// rpcserverip rpcserverport zookeeperip zookeeperport
class MprpcConfig
{
public:
    // 负责解析加载配置文件
    void LoadConfigFile(const char *config_file);
    // 查询配置项信息
    std::string Load(std::string key);

private:
    std::unordered_map<std::string, std::string> m_configMap;
    // 用于在配置文件中读取配置项时,整理格式
    void Trim(std::string &src_buf);
};