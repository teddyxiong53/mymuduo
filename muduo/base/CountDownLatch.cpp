#include "CountDownLatch.h"

namespace muduo
{
CountDownLatch::CountDownLatch(int count)
 : m_mutex(),
   m_cv(),
   m_count(count)
{

}

void CountDownLatch::wait()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_count > 0)
    {
        m_cv.wait(lock);
    }
}

void CountDownLatch::countDown()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_count --;
    if(m_count == 0) {
        m_cv.notify_all();
    }
}

int CountDownLatch::getCount()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_count;
}

} // namespace muduo

