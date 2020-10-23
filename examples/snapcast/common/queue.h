#pragma once

#include <deque>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

template<typename T>
class Queue
{
public:
    Queue() = default;
    Queue(const Queue&) = delete;//禁止拷贝构造和赋值。
    Queue& operator=(const Queue&) = delete;

    //push
    void push(const T& item)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push_back(item);
        }
        m_cond.notify_one();
    }
    void push(T&& item)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push_back(std::move(item));
        }
        m_cond.notify_one();
    }
    void push_front(const T& item)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push_front(item);
        }
        m_cond.notify_one();
    }
    void push_front(T&& item)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push_front(std::move(item));
        }
        m_cond.notify_one();
    }

    //pop
    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_queue.empty()) {
            m_cond.wait(lock);
        }
        auto val = m_queue.front();
        m_queue.pop_front();
        return val;
    }
    void pop(T& item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_queue.empty()) {
            m_cond.wait(lock);
        }
        item = m_queue.front();
        m_queue.pop_front();
    }
    //毫秒为单位，
    bool try_pop(T& item, std::chrono::milliseconds timeout)
    {
        return try_pop(item, std::chrono::duration_cast<std::chrono::microseconds>(timeout));
    }
    //微秒为单位。
    bool try_pop(T& item, std::chrono::microseconds timeout)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_abort = false;
        if(!m_cond.wait_for(
            lock,
            timeout,
            [this]() {
                return (!m_queue.empty() || m_abort);
            }
        ))
        {
            return false;
        }
        if(m_queue.empty() || m_abort) {
            return false;
        }
        item = std::move(m_queue.front());
        m_queue.pop_front();
        return true;


    }
    //wait
    bool wait_for(std::chrono::microseconds timeout)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_abort = false;
        if(!m_cond.wait_for(
            lock,
            timeout,
            [this]() {
                return (!m_queue.empty() || m_abort);
            }
        ))
        {
            return false;
        }
        if(m_queue.empty() || m_abort) {
            return false;
        }
    }
    //放弃等待
    void abort_wait()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_abort = true;
        }
        m_cond.notify_one();
    }
    //查询
    T front()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_queue.empty()) {
            m_cond.wait(lock);
        }
        return m_queue.front();
    }
    size_t size()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
    bool empty()
    {
        return (size() == 0);
    }
private:
    std::deque<T> m_queue;
    std::atomic<bool> m_abort;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};
