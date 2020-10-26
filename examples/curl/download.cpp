#include "Curl.h"
#include "muduo/net/EventLoop.h"
#include <sstream>

#include "mylog.h"
using namespace muduo;
using namespace muduo::net;

typedef std::shared_ptr<FILE> FilePtr;

template<int N>
bool startWith(const std::string& str, const char (&prefix)[N])
{
    return str.size() >= N-1 &&
        std::equal(
            prefix,
            prefix+N-1,
            str.begin()
        );
}
class Piece: noncopyable
{
public:
    Piece(
        const curl::RequestPtr& req,
        const FilePtr& out,
        const std::string& range,
        std::function<void()> done
    )
     : m_req(req),
        m_out(out),
        m_range(range),
        m_doneCb(done)
    {
        m_req->setRange(range);
        m_req->setDataCallback(
            std::bind(&Piece::onData, this, _1, _2)
        );
        m_req->setDoneCallback(
            std::bind(&Piece::onDone, this, _1, _2)
        );
    }
private:
    void onData(const char *data, int len)
    {
        ::fwrite(data, 1, len, m_out.get());
    }
    void onDone(curl::Request *c, int code)
    {
        m_req.reset();
        m_out.reset();
        m_doneCb();
    }
private:
    curl::RequestPtr m_req;
    FilePtr m_out;
    std::string m_range;
    std::function<void()> m_doneCb;
};
class Downloader : muduo::noncopyable
{
public:
    Downloader(EventLoop* loop, std::string& url)
     : m_loop(loop),
       m_curl(m_loop),
       m_url(url),
       m_req(m_curl.getUrl(m_url)),
       m_found(false),
       m_acceptRanges(false),
       m_length(0),
       m_pieces(kConcurrent),
       m_concurrent(0)
    {
        m_req->setHeaderCallback(
            std::bind(&Downloader::onHeader, this, _1, _2)
        );
        m_req->setDoneCallback(
            std::bind(&Downloader::onHeaderDone, this,_1,_2 )
        );
        m_req->headerOnly();
    }
private:
    void onHeader(const char *data, int len)
    {
        std::string line(data, len);
        if(startWith(line, "HTTP/1.1 200")
            || startWith(line, "HTTP/1.0 200"))
        {
            m_found = true;
            mylogd("");
        }
        if(line == "Accept-Ranges: bytes\r\n")
        {
            m_acceptRanges = true;
            mylogd("");
        }
        else if(startWith(line, "Content-Length:"))
        {
            m_length = atoll(line.c_str() + strlen("Content-Length:"));
            mylogd("m_length:%lld", m_length);
        }
    }
    void onHeaderDone(curl::Request* req, int code)
    {
        if(m_acceptRanges && m_length > kConcurrent*4096) {
            mylogd("download with %d connections", kConcurrent);
            m_concurrent = kConcurrent;
            concurrentDownload();
        } else if(m_found) {
            //单线程下载
            mylogd("single connection download");
            FILE *fp = ::fopen("output", "wb");
            if(fp) {
                FilePtr(fp, ::fclose).swap(m_out);
                m_req.reset();
                m_req2 = m_curl.getUrl(m_url);
                m_req2->setDataCallback(
                    std::bind(&Downloader::onData, this, _1, _2)
                );
                m_req2->setDoneCallback(
                    std::bind(
                        &Downloader::onDownloadDone, this
                    )
                );
                m_concurrent = 1;
            } else {
                myloge("can not  create output file");
                m_loop->quit();
            }
        } else {
            myloge("file not found");
            m_loop->quit();
        }
    }
    void onData(const char *data, int len)
    {
        ::fwrite(data, 1, len, m_out.get());
    }
    void onDownloadDone()
    {
        if(--m_concurrent <= 0) {
            m_loop->quit();
        }
    }
    void concurrentDownload()
    {
        const int64_t pieceLen = m_length/kConcurrent;
        for(int i=0; i<kConcurrent; i++) {
            char buf[256] = {0};
            snprintf(buf, sizeof(buf), "output-%05d-of-%-5d", i, kConcurrent);
            FILE *fp = ::fopen(buf, "wb");
            if(fp) {
                FilePtr out(fp, ::fclose);
                curl::RequestPtr req = m_curl.getUrl(m_url);
                std::ostringstream range;
                if(i< kConcurrent -1) {
                    range << i*pieceLen << "-" << (i+1)*pieceLen - 1;
                } else {
                    range << 8*pieceLen << "-" << m_length -1;
                }
                m_pieces[i].reset(
                    new Piece(
                        req,
                        out,
                        range.str(),
                        std::bind(
                            &Downloader::onDownloadDone, this
                        )
                    )
                );
            } else {
                myloge("can not create output file:%s", buf);
                m_loop->quit();
            }
        }
    }
private:
    EventLoop *m_loop;
    curl::Curl m_curl;
    std::string m_url;
    curl::RequestPtr m_req;
    curl::RequestPtr m_req2;
    bool m_found;
    bool m_acceptRanges;
    int64_t m_length;
    FilePtr m_out;
    std::vector<std::unique_ptr<Piece>> m_pieces;
    int m_concurrent;
    const static int kConcurrent = 4;
};
int main(int argc, char const *argv[])
{
    EventLoop loop;
    curl::Curl::initialize(curl::Curl::Option::kCURLssl);
    std::string url;
    if(argc > 1) {
        std::string url = argv[1];
    } else {
        printf("usage: ./download url\n");
        exit(1);
    }
    Downloader d(&loop, url);
    loop.loop();
    return 0;
}
