#pragma once

#include "muduo/base/noncopyable.h"
#include "muduo/base/StringPiece.h"
#include "muduo/net/Callbacks.h"
#include <memory>

extern "C"
{
typedef void CURLM;
typedef void CURL;
}

namespace muduo
{
namespace net
{
class Channel;
class EventLoop;
} // namespace net

} // namespace muduo

namespace curl
{
class Curl;

class Request: public std::enable_shared_from_this<Request>, muduo::noncopyable
{
public:
    typedef std::function<void (const char *, int)> DataCallback;
    typedef std::function<void (Request*, int)> DoneCallback;

    Request(Curl *, const char *ulr);
    ~Request();

    template<typename OPT>
    int setopt(OPT opt, long p)
    {
        return curl_easy_setopt(m_curl, opt, p);
    }
    template<typename OPT>
    int setopt(OPT opt, const char *p)
    {
        return curl_easy_setopt(m_curl, opt, p);
    }
    template<typename OPT>
    int setopt(OPT opt, void *p)
    {
        return curl_easy_setopt(m_curl, opt, p);
    }
    template <typename OPT>
    int setopt(OPT opt, size_t (*p)(char *, size_t ,size_t, void *))
    {
        return curl_easy_setopt(m_curl, opt, p);
    }

    void setDataCallback(const DataCallback& cb)
    {
        m_dataCb = cb;
    }
    void setDoneCallback(const DoneCallback& cb)
    {
        m_doneCb = cb;
    }
    void setHeaderCallback(const DataCallback &cb)
    {
        m_headerCb = cb;
    }
    void headerOnly();
    void setRange(const muduo::StringArg range);
    const char *getEffectiveUrl();
    const char *getRedirectUrl();
    int getResponseCode();
    muduo::net::Channel *setChannel(int fd);
    void removeChannel();
    void done(int code);
    CURL *getCurl() {
        return m_curl;
    }
    muduo::net::Channel* getChannel()
    {
        m_channel.get();
    }
private:
    void dataCallback(const char *buffer, int len);
    void headerCallback(const char *buffer, int len);
    static size_t writeData(
        char *buffer,
        size_t size,
        size_t nmemb,
        void *userp
    );
    static size_t headerData(
        char *buffer,
        size_t size,
        size_t nmemb,
        void *userp
    );
    void doneCallback();
private:
    class Curl* m_owner;//特意强调是class Curl，避免把struct Curl解析过来。
    CURL *m_curl;
    std::shared_ptr<muduo::net::Channel> m_channel;
    DataCallback m_dataCb;
    DataCallback m_headerCb;
    DoneCallback m_doneCb;

};
typedef std::shared_ptr<Request> RequestPtr;

class Curl: muduo::noncopyable
{
public:
    enum Option
    {
        kCURLnossl = 0,
        kCURLssl = 1
    };
    explicit Curl(muduo::net::EventLoop *loop);
    ~Curl();
    CURLM *getCurlm() {
        return m_curlm;
    }
    static void initialize(Option opt=kCURLnossl);
    RequestPtr getUrl(muduo::StringArg url);
    muduo::net::EventLoop* getLoop()
    {
        return m_loop;
    }

private:
    void onTimer();
    void onRead(int fd);
    void onWrite(int fd);
    void checkFinish();
    static int socketCallback(CURL*, int, int, void*, void*);
    static int timerCallback(CURLM*, long, void*);

    CURLM *m_curlm;
    muduo::net::EventLoop *m_loop;
    int m_runningHandlers;
    int m_prevRunningHandlers;
};//class Curl
} // namespace curl


