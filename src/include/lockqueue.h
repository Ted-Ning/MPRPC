#pragma once

#include <queue>
#include <thread>
#include <mutex>              //pthread_mutex_t
#include <condition_variable> //pthread_condition_t

// 模版代码,不能分头文件和源文件
template <typename T>
class lockqueue
{
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;

public:
    // 多个工作线程都会写日志queue
    void Push(const T &data)
    {
        // 给消息队列加锁,向消息队列写日志
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        // 通知磁盘IO线程,读消息队列,因为只有一个线程负责磁盘IO,所以此处用one,多个线程可改all
        m_cv.notify_one();
    }

    // 一个线程读日志queue,写IO日志
    T Pop()
    {
        // 加锁
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            // 日志队列为空,线程进入wait状态
            m_cv.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }
};
