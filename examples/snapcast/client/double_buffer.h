#pragma once

#include <algorithm>
#include <deque>

template <typename T>
class DoubleBuffer
{
public:
    DoubleBuffer(size_t size = 10): m_bufferSize(size)
    {

    }
    inline void add(const T& element)
    {
        m_buffer.push_back(element);
        if(m_buffer.size() > m_bufferSize) {
            m_buffer.pop_front();
        }
    }
    inline void add(T&& element)
    {
        m_buffer.push_back(std::move(element));
        if(m_buffer.size() > m_bufferSize) {
            m_buffer.pop_front();
        }
    }
    //取得中间值
    T median(unsigned int mean = 1)
    {
        if(m_buffer.empty()) {
            return 0;
        }
        std::deque<T> tmpBuffer(m_buffer.begin(), m_buffer.end());
        std::sort(tmpBuffer.begin(), tmpBuffer.end());
        if((mean < 1)
            || (tmpBuffer.size() < mean)
        )
        {
            return tmpBuffer[tmpBuffer.size()/2];
        }
        else
        {
            unsigned int low = tmpBuffer.size()/2;
            unsigned int high = low;
            low -= mean/2;
            high += mean/2;
            T result(T(0));
            for(unsigned int i=low; i<=high; i++) {
                result += tmpBuffer[i];
            }
            return result/mean;
        }
    }
    //取得平均值
    double mean()
    {
        if(m_buffer.empty()) {
            return 0;
        }
        double mean = 0.0;
        for(size_t n=0; n<m_buffer.size(); n++) {
            mean += (float)m_buffer[n]/(float)m_buffer.size();
        }
        return mean;
    }
    //取得百分比
    T percentile(unsigned int percentile)
    {
        if(m_buffer.empty()) {
            return 0;
        }
        std::deque<T> tmpBuffer(m_buffer.begin(), m_buffer.end());
        std::sort(tmpBuffer.begin(), tmpBuffer.end());
        return tmpBuffer[(size_t)(tmpBuffer.size() * ((float)percentile / (float)100))];
    }
    inline bool full()
    {
        return m_buffer.size() == m_bufferSize;
    }
    inline void clear()
    {
        m_buffer.clear();
    }
    inline size_t size()
    {
        return m_buffer.size();
    }
    inline bool empty()
    {
        return m_buffer.size() == 0;
    }
    void setSize(size_t size)
    {
        m_bufferSize = size;
    }
    const std::deque<T>& getBuffer()
    {
        return &m_buffer;
    }
private:
    size_t m_bufferSize;
    std::deque<T> m_buffer;
};
