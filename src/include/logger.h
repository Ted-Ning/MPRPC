#pragma once
#include "lockqueue.h"
#include <string>

enum LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
};

// Mprpc框架提供的日志系统,以单例模式设计
class Logger
{
public:
    // 获取日志的单例
    static Logger &GetInstance();
    // 设置日志级别
    void SetLogLevel(LogLevel level);
    // 写日志
    void Log(std::string msg);
    // 原方法存在缺陷,重载新的日志写入方法
    void Log(std::string msg, std::string logLevel);

private:
    int m_loglevel;                  // 记录日志级别
    lockqueue<std::string> m_lckQue; // 日志缓冲队列
    // 用pair存储二元值,不影响lockqueue的底层实现
    lockqueue<std::pair<std::string, std::string>> m_lckQue2; // 日志缓冲队列

    Logger();
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
};

/*
    为方便用户使用日志模块,定义宏
    反斜杠是连接符,防止define定义过长,可读性差
    ##__VA__ARGS__是接收可变参的一种特殊写法
*/

// 存在缺陷,弃用
//  // 普通信息日志
//  #def ine  LOG_INFO(logmsgformat, ...)                          \
//     do                                                       \
//     {                                                        \
//         Logger &logger = Logger::GetInstance();              \
//         logger.SetLogLevel(INFO);                            \
//         char logmsg[1024] = {0};                             \
//         snprintf(logmsg, 1024, logmsgformat, ##__VA_ARGS__); \
//         logger.Log(logmsg);                                  \
//     } while (0)

// // 错误信息日志
// #define LOG_ERR(logmsgformat, ...)                           \
//     do                                                       \
//     {                                                        \
//         Logger &logger = Logger::GetInstance();              \
//         logger.SetLogLevel(ERROR);                           \
//         char logmsg[1024] = {0};                             \
//         snprintf(logmsg, 1024, logmsgformat, ##__VA_ARGS__); \
//         logger.Log(logmsg);                                  \
//     } while (0)

// 普通信息日志
#define LOG_INFO(logmsgformat, ...)                          \
    do                                                       \
    {                                                        \
        Logger &logger = Logger::GetInstance();              \
        char logmsg[1024] = {0};                             \
        snprintf(logmsg, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(logmsg, "INFO");                          \
    } while (0)

// 错误信息日志
#define LOG_ERR(logmsgformat, ...)                           \
    do                                                       \
    {                                                        \
        Logger &logger = Logger::GetInstance();              \
        char logmsg[1024] = {0};                             \
        snprintf(logmsg, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(logmsg, "ERROR");                         \
    } while (0)
