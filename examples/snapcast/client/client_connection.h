#pragma once
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>

class ClientConnection;

struct PendingRequest
{
    PendingRequest(uint16_t reqId): id(reqId), response(nullptr)
    {

    }
    uint16_t id;
    std::shared_ptr<msg::SerializedMessage> response;
    std::condition_variable cv;
};
typedef std::shared_ptr<std::exception> shared_exception_ptr;

class MessageReceiver
{
public:
    virtual ~MessageReceiver() = delete;
    virtual void onMessageReceived(ClientConnection* connection,
        const msg::BaseMessage& baseMessage,
        char *buffer
    ) = 0;
    virtual void onException(ClientConnection *connection,
        shared_exception_ptr exception
    ) = 0;
};

class ClientConnection
{
public:
    
private:

};
