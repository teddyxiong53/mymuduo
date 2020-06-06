#ifndef _MUDUO_EX_CHAT_CODEC_H_
#define _MUDUO_EX_CHAT_CODEC_H_

#include "muduo/net/TcpConnection.h"
#include "muduo/base/noncopyable.h"
#include "muduo/net/Buffer.h"
#include <functional>
#include <arpa/inet.h>
#include "mylog.h"

class LengthHeaderCodec: muduo::noncopyable
{
public:
    typedef std::function<void (
        const muduo::net::TcpConnectionPtr &conn,
        const std::string& message,
        muduo::Timestamp receiveTime
    )> StringMessageCallback;
    explicit LengthHeaderCodec(const StringMessageCallback &cb)
     : m_messageCallback(cb)
    {

    }
    void onMessage(const muduo::net::TcpConnectionPtr& conn,
        muduo::net::Buffer* buf,
        muduo::Timestamp receiveTime)
    {
        while(buf->readableBytes() >= kHeaderLen) {
            //收到了至少包头长度的数据。
            const void *data = buf->peek();
            int32_t be32 = *(int32_t *)data;
            const int32_t len = ntohl(be32);
            if(len > 65536 || len < 0) {
                myloge("invalid len:%d", len);
                conn->shutdown();
                break;
            } else if(buf->readableBytes() >= len+kHeaderLen) {
                //这个长度是合法的。
                buf->retrieve(kHeaderLen);//先取包头。
                std::string message(buf->peek(), len);
                m_messageCallback(conn, message, receiveTime);
                buf->retrieve(len);//取消息。
            } else {
                break;//这个说明长度不对。
            }

        }
    }
    void send(muduo::net::TcpConnection* conn, const muduo::StringPiece& message)
    {
        muduo::net::Buffer buf;
        buf.append(message.data(), message.size());
        int32_t len = (int32_t)message.size();
        int32_t be32 = htonl(len);
        buf.prepend(&be32, sizeof(be32));
        conn->send(&buf);
    }
private:
    StringMessageCallback m_messageCallback;
    const static size_t kHeaderLen = sizeof(int32_t);
};
#endif
