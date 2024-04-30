// 防止头文件重复包含
#pragma once

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

// mprpc框架的基础类,采用单例模式设计
class MprpcApplicaiton
{
public:
    // 静态初始化函数
    static void Init(int argc, char **argv);
    // 获取单例的唯一接口
    static MprpcApplicaiton &GetInstance();
    static MprpcConfig &GetConfig();

private:
    static MprpcConfig m_config;

    MprpcApplicaiton() {}
    // 这一行使用 = delete 声明了复制构造函数被删除。
    // 这意味着编译器将禁止生成类的对象的副本，即禁止使用拷贝构造函数来复制对象。
    MprpcApplicaiton(const MprpcApplicaiton &) = delete;
    // 这一行使用 = delete 声明了移动构造函数被删除。
    // 这意味着编译器将禁止生成类的对象的移动副本，即禁止使用移动构造函数来移动对象。
    MprpcApplicaiton(MprpcApplicaiton &&) = delete;
};