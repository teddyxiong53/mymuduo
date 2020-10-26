#include "Curl.h"
#include "mylog.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/Channel.h"
#include <curl/curl.h>
using namespace muduo;
using namespace muduo::net;
using namespace curl;

static void dummy(const std::shared_ptr<Channel> &)
{

}

Request::Request(Curl *owner, const char *url)
 : m_owner(owner),
   m_curl(curl_easy_init())
{
    setopt(CURLOPT_URL, url);
    setopt(CURLOPT_WRITEFUNCTION, &Request::writeData);
    setopt(CURLOPT_WRITEDATA, this);
    setopt(CURLOPT_HEADERFUNCTION, &Request::headerData);
    setopt(CURLOPT_HEADERDATA, this);
    setopt(CURLOPT_PRIVATE, this);
    setopt(CURLOPT_USERAGENT, "curl");
    curl_multi_add_handle(m_owner->getCurlm(), m_curl);
}

Request::~Request()
{
    curl_multi_remove_handle(m_owner->getCurlm(), m_curl);
    curl_easy_cleanup(m_curl);
}

void Request::headerOnly()
{
    setopt(CURLOPT_NOBODY ,1);
}

void Request::setRange(const muduo::StringArg range)
{
    // setopt(CURLOPT_RANGE, range.c_str());
}

const char *Request::getEffectiveUrl()
{
    const char *p = NULL;
    curl_easy_getinfo(m_curl, CURLINFO_EFFECTIVE_URL, &p);
    return p;
}

const char *Request::getRedirectUrl()
{
    const char *p = NULL;
    curl_easy_getinfo(m_curl, CURLINFO_REDIRECT_URL, &p);
    return p;
}

int Request::getResponseCode()
{
    long code = 0;
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &code);
    return (int)code;
}
Channel * Request::setChannel(int fd)
{
    m_channel.reset(new Channel(m_owner->getLoop(), fd));
    m_channel->tie(shared_from_this());
    return m_channel.get();
}
void Request::removeChannel()
{
    m_channel->disableAll();
    m_channel->remove();
    m_owner->getLoop()->queueInLoop(
        std::bind(&dummy, m_channel)
    );
    m_channel.reset();
}

void Request::done(int code)
{
    if(m_doneCb) {
        m_doneCb(this, code);
    }
}

void Request::dataCallback(const char *buffer, int len)
{
    if(m_dataCb) {
        m_dataCb(buffer, len);
    }
}

void Request::headerCallback(const char *buffer, int len)
{
    if(m_headerCb) {
        m_headerCb(buffer, len);
    }
}
size_t Request::writeData(
    char *buffer,
    size_t size,
    size_t nmemb,
    void *userp
)
{
    Request *req = (Request *)userp;
    req->dataCallback(buffer, (int)nmemb);
    return nmemb;
}

size_t Request::headerData(
    char *buffer,
    size_t size,
    size_t nmemb,
    void *userp
)
{
    Request *req = (Request *)userp;
    req->headerCallback(buffer, (int)nmemb);
    return nmemb;
}

// void doneCallback();

Curl::Curl(muduo::net::EventLoop *loop)
 : m_loop(loop),
   m_curlm(curl_multi_init()),
   m_runningHandlers(0),
   m_prevRunningHandlers(0)
{
    curl_multi_setopt(m_curlm, CURLMOPT_SOCKETFUNCTION, &Curl::socketCallback);
    curl_multi_setopt(m_curlm, CURLMOPT_SOCKETDATA, this);
    curl_multi_setopt(m_curlm, CURLMOPT_TIMERFUNCTION, &Curl::timerCallback);
    curl_multi_setopt(m_curlm, CURLMOPT_TIMERDATA, this);
}

Curl::~Curl()
{
    curl_multi_cleanup(m_curlm);
}

void Curl::initialize(Option opt)
{
    curl_global_init(
        opt == kCURLnossl? CURL_GLOBAL_NOTHING : CURL_GLOBAL_SSL
    );
}
int Curl::socketCallback(
    CURL* c,
    int fd,
    int what,
    void *userp,
    void *socketp
)
{
    Curl *curl = static_cast<Curl*>(userp);
    const char *whatstr[] = {
        "none",
        "IN",
        "OUT",
        "INOUT",
        "REMOVE"
    };
    mylogd("fd:%d, what:%s", fd, whatstr[what]);
    Request *req = NULL;
    curl_easy_getinfo(c, CURLINFO_PRIVATE, &req);
    if(what == CURL_POLL_REMOVE) {//移除
        muduo::net::Channel *ch = static_cast<Channel*>(socketp);
        req->removeChannel();
        ch = NULL;
        curl_multi_assign(curl->m_curlm, fd, ch);
        mylogd("remove");
    } else {//添加
        muduo::net::Channel *ch = (Channel *)socketp;
        if(!ch) {
            ch->setReadCallback(std::bind(&Curl::onRead, curl, fd));
            ch->setWriteCallback(std::bind(&Curl::onWrite, curl, fd));
            ch->enableReading();
            curl_multi_assign(curl->m_curlm, fd, ch);
            mylogd("add ");
        }
        //这个是更新状态
        if(what & CURL_POLL_OUT) {
            ch->enableWriting();
        } else {
            ch->disableWriting();
        }


    }
    return 0;
}
int Curl::timerCallback(
    CURLM *curlm,
    long ms,
    void *userp
)
{
    Curl *curl = (Curl *)userp;
    curl->m_loop->runAfter(
        (int )ms/1000.0,
        std::bind(&Curl::onTimer, curl)
    );
    return 0;
}

RequestPtr Curl::getUrl(muduo::StringArg url)
{
    RequestPtr req(new Request(this, url.c_str()));
    return req;
}

void Curl::onTimer()
{
    CURLMcode rc = CURLM_OK;
    do {
        mylogd("");
        rc = curl_multi_socket_action(
            m_curlm,
            CURL_SOCKET_TIMEOUT,
            0,
            &m_runningHandlers
        );
    } while(rc == CURLM_CALL_MULTI_PERFORM);
    checkFinish();
}

void Curl::checkFinish()
{
    if(m_prevRunningHandlers > m_runningHandlers) {
        CURLMsg *msg = NULL;
        int left = 0;
        while((msg = curl_multi_info_read(m_curlm, &left)) != NULL) {
            if(msg->msg == CURLMSG_DONE) {
                CURL *c = msg->easy_handle;
                CURLcode res = msg->data.result;
                Request *req = NULL;
                curl_easy_getinfo(c, CURLINFO_PRIVATE, &req);
                mylogd("");
                req->done(res);
            }
        }
    }
    m_prevRunningHandlers = m_runningHandlers;
}

void Curl::onRead(int fd)
{
    CURLMcode rc = CURLM_OK;
    do {
        mylogd("fd:%d", fd);
        rc = curl_multi_socket_action(
            m_curlm,
            fd,
            CURL_POLL_IN,
            &m_runningHandlers
        );
        mylogd("running handlers:%d", m_runningHandlers);
    } while(rc == CURLM_CALL_MULTI_PERFORM);
    checkFinish();
}

void Curl::onWrite(int fd)
{
    CURLMcode rc = CURLM_OK;
    do {
        mylogd("fd:%d" ,fd);
        rc = curl_multi_socket_action(
            m_curlm,
            fd,
            CURL_POLL_OUT,
            &m_runningHandlers
        );
        mylogd("running handlers:%d", m_runningHandlers);
    } while(rc == CURLM_CALL_MULTI_PERFORM);
    checkFinish();
}

