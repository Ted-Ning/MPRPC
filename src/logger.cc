#include "logger.h"
#include <time.h>
#include <iostream>

Logger::Logger()
{
    // 启动专门的写日志线程
    std::thread writeLogTask([&]()
                             {
        for(;;)
        { 
            //
            // 原日志系统设计存在缺陷
            // //获取当前 的日期,然后取日志信息,写入响应的日志文件当中
            // time_t now = time(nullptr);
            // tm* nowtm = localtime(&now);
            // char file_name[128];
            // sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);
            // /*
            //     a+" 模式：
            //     以追加读写方式打开文件。如果文件不存在，则创建文件；如果文件已存在，则在文件末尾追加内容。
            //     文件指针位于文件的末尾。
            //     支持读取和写入文件内容，不会覆盖原有内容。
            // */
            // FILE *pf = fopen(file_name,"a+");
            // if(pf == nullptr)
            // {
            //     std::cout << " logger file: " << file_name << " open error! " << std::endl;
            //     exit(EXIT_FAILURE);
            // }

            // std::string msg = m_lckQue.Pop();
            // //为日志信息加上时间戳
            // char time_buf[128] = {0};
            // sprintf(time_buf," %d:%d:%d=>[%s] ",
            //                     nowtm->tm_hour,
            //                     nowtm->tm_min,
            //                     nowtm->tm_sec,
            //                     (m_loglevel == INFO) ? "INFO" : "ERROR");

            // msg.insert(0,time_buf);
            // msg.append("\n");

            // fputs(msg.c_str(), pf);
            // fclose(pf);  


            //获取当前 的日期,然后取日志信息,写入响应的日志文件当中

            time_t now = time(nullptr);
            tm* nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);

            /*
                a+" 模式：
                以追加读写方式打开文件。如果文件不存在，则创建文件；如果文件已存在，则在文件末尾追加内容。
                文件指针位于文件的末尾。
                支持读取和写入文件内容，不会覆盖原有内容。
            */
            FILE *pf = fopen(file_name,"a+");
            if(pf == nullptr)
            {
                std::cout << " logger file: " << file_name << " open error! " << std::endl;
                exit(EXIT_FAILURE);
            }

            //std::string msg = m_lckQue.Pop();
            auto msg = m_lckQue2.Pop();
            //为日志信息加上时间戳
            char time_buf[128] = {0};
            // sprintf(time_buf," %d:%d:%d=>[%s] ",
            //                     nowtm->tm_hour,
            //                     nowtm->tm_min,
            //                     nowtm->tm_sec,
            //                     (m_loglevel == INFO) ? "INFO" : "ERROR");
            //std::cout<<msg.first<<std::endl;
            sprintf(time_buf," %d:%d:%d=>[%s] ",
                        nowtm->tm_hour,
                        nowtm->tm_min,
                        nowtm->tm_sec,
                        msg.first.c_str());

            // msg.insert(0,time_buf);
            // msg.append("\n");
            msg.second.insert(0,time_buf);
            msg.second.append("\n");

            // fputs(msg.c_str(), pf);
            fputs(msg.second.c_str(), pf);
            fclose(pf);  
        } });
    // 设置分离线程,守护线程
    writeLogTask.detach();
}

// 获取日志的单例
Logger &Logger::GetInstance()
{
    static Logger logger;
    return logger;
}
// 设置日志级别
void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel = level;
}
// 写日志,把日志信息写入lockqueue缓冲区中
void Logger::Log(std::string msg)
{
    m_lckQue.Push(msg);
}

// 原方法存在缺陷,重载新的日志写入方法
void Logger::Log(std::string msg, std::string loglevel)
{
    m_lckQue2.Push({loglevel, msg});
}