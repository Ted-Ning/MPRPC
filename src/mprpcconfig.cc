#include "mprpcconfig.h"
#include <iostream>
#include <string>

// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    // 以只读形式打开配置文件
    FILE *pf = fopen(config_file, "r");
    if (pf == nullptr)
    {
        std::cout << config_file << " is not exits" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 读取配置文件需要处理的内容 1.注释 2.正确配置项 3.空格
    while (!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);
        std::string read_buf(buf);
        Trim(read_buf);

        // 判断#的注释
        if (read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }

        // 解析配置项
        int idx = read_buf.find('=');
        if (idx == -1)
        {
            // 配置不合法
            continue;
        }

        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);
        // 对读取到的 key 和 value 再分别去除一次首位多余空格
        Trim(key);
        // 找到每行数据末尾的换行符
        int endidx = read_buf.find('\n', idx);
        // 忽略末尾的换行符,否则无法去除value与换行符之间的空格
        value = read_buf.substr(idx + 1, endidx - idx - 1);
        Trim(value);
        m_configMap.insert({key, value});
    }
}

// 查询配置项信息
std::string MprpcConfig::Load(std::string key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}

void MprpcConfig::Trim(std::string &src_buf)
{
    // 去掉字符串前面多余的空格
    int idx = src_buf.find_first_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串前面有空格,去除空格
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }
    // 去掉字符串后面多余的空格
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串后面有空格,去除空格
        src_buf = src_buf.substr(0, idx + 1);
    }
}