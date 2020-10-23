#include "controller.h"
#include "mylog.h"
#include "common/utils.h"
#include "common/message/hello.h"
#include <unistd.h>
#include "common/message/time.h"
#include "common/message/server_settings.h"
#include <memory>
#include "client/decoder/pcm_decoder.h"
#include "common/snap_exception.h"
#include "common/message/stream_tags.h"
#include "client/player/alsa_player.h"

Controller::Controller(EventLoop* loop, const InetAddress& addr)
: m_client(loop, addr),
  m_pcmDevice(),
  m_controllerThread(std::bind(&Controller::worker, this), "controller")
    //线程必须放在这里构造。放其他地方都报错的。
{
    m_instance = 1;
    m_active = false;

    m_client.setConnectionCallback(
        std::bind(&Controller::onConnection, this, _1)
    );
    m_client.setMessageCallback(
        std::bind(&Controller::onMessage, this, _1, _2, _3)
    );
    m_client.enableRetry();
}
void Controller::connect()
{
    m_client.connect();
}
void Controller::disconnect()
{
    m_client.disconnect();
}

void Controller::sendMessage(msg::BaseMessage* message) {
    // std::unique_lock<std::mutex> lock(m_mutex);
    //消息得到的是一个ostream。需要把这个ostream转成string
    std::ostringstream oss;
    tv t;
    message->sent = t;
    message->serialize(oss);
    if(m_connection) {
        m_connection->send(oss.str().data(), oss.tellp());
    }
}

void Controller::onConnection(const TcpConnectionPtr & conn) {
    if(conn->connected()) {
        mylogd("connected");
        m_connection = conn;
        //连接建立后，发送hello消息
        //拿到mac地址
        std::string macAddress = ::getMacAddress(m_connection->getSocketFd());
        //把mac地址作为hostId
        m_hostId = ::getHostId(macAddress);
        msg::Hello hello(macAddress, m_hostId, m_instance);
        sendMessage(&hello);

    } else {
        m_connection.reset();
    }
}
void Controller::onMessage(const TcpConnectionPtr& conn,
    Buffer* buf,
    muduo::Timestamp receiveTime)
{
    // printf("[%s]>> %s\n", receiveTime.toFormattedString().c_str(), buf->retrieveAsString().c_str());
    //现在消息是在Buffer里。
    //取出来。解析成BaseMessage才行。
    //具体做法是：
    /*
    先构造一个BaesMessage
    */
    msg::BaseMessage baseMessage;
    size_t baseMsgSize = baseMessage.getSize();
    //先读取基本长度
    std::string base = buf->retrieveAsString(baseMsgSize);
    //之所以要借助vector，因为deserialize只接收char*，不接收const char *
    std::vector<char> buffer(base.begin(), base.end());
    //解析消息内容
    baseMessage.deserialize(buffer.data());
    //根据取出长度值，继续读取剩余的部分
    std::string realMsg = buf->retrieveAsString(baseMessage.size);
    buffer.assign(realMsg.begin(), realMsg.end());
    tv t;
    baseMessage.received = t;
    //TODO 这里省略一些代码，后面再补充。
    //下面就分情况进行处理。
    if(baseMessage.type == message_type::kTime) {
        //时间消息处理。
        //时间都是client发送，server回复。
        msg::Time reply;
        reply.deserialize(baseMessage, &buffer[0]);
        //计算时间差。
    } else if(baseMessage.type == message_type::kCodecHeader) {
        //这个消息，会触发一些类的创建。
        m_headerChunk.reset(new msg::CodecHeader());
        m_headerChunk->deserialize(baseMessage, &buffer[0]);
        mylogd("codec name:%s", m_headerChunk->codec.c_str());
        //
        m_decoder.reset(nullptr);
        m_stream = nullptr;
        m_player.reset(nullptr);
        if(m_headerChunk->codec == "pcm") {
            m_decoder = std::make_unique<decoder::PcmDecoder>();
        } else {
            throw SnapException("codec not support");
        }
        m_sampleFormat = m_decoder->setHeader(m_headerChunk.get());
        m_stream = std::make_shared<Stream>(m_sampleFormat);
        m_stream->setBufferLen(m_serverSettings->getBufferMs() - m_latency);

        m_player = std::make_unique<AlsaPlayer>(m_pcmDevice, m_stream);
        m_player->setVolume(m_serverSettings->getVolume()/100.0);
        m_player->setMute(m_serverSettings->isMuted());
        m_player->start();
    } else if(baseMessage.type == message_type::kServerSettings) {
        m_serverSettings.reset(new msg::ServerSettings());
        m_serverSettings->deserialize(baseMessage, &buffer[0]);
        mylogd("server settings:");

    } else if(baseMessage.type == message_type::kStreamTags) {
        //这个是做什么呢？
        //stream tag，是表示stream的一些信息，例如从哪里过来的。
        m_streamTags.reset(new msg::StreamTags());
        m_streamTags->deserialize(baseMessage, &buffer[0]);
    } else if(baseMessage.type == message_type::kWireChunk){
        //这个分支是最重要的。处理音频数据。
        //加入到stream里，
        if(m_stream && m_decoder) {
            auto *pcmChunk = new msg::PcmChunk(m_sampleFormat, 0);
            pcmChunk->deserialize(baseMessage, &buffer[0]);
            if(m_decoder->decode(pcmChunk)) {
                m_stream->addChunk(pcmChunk);
            } else {
                delete pcmChunk;
            }
        }
    }
    //最后，如果不是时间消息，那么就同步一次时间。
    if(baseMessage.type != message_type::kTime) {
        sendTimeSyncMessage(1000);
    }
}

void Controller::start()
{
    m_active = true;
    connect();
    m_controllerThread.start();
}
void Controller::stop()
{
    m_active =false;
    m_controllerThread.join();
}
void Controller::worker()
{
    while(m_active) {
        sleep(1);
        // mylogd("");
        //每5秒，发送一次时间同步消息。
        sendTimeSyncMessage(5000);
    }
}

bool Controller::sendTimeSyncMessage(long after)
{
    static long lastTimeSync = 0;
    long now = chronos::getTickCount();
    if(lastTimeSync + after > now) {
        return false;//时间还没有到。
    }
    lastTimeSync = now;
    msg::Time timeReq;
    sendMessage(&timeReq);


}
