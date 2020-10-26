#pragma once

#include "common/json.hpp"
#include "message.h"
#include "mylog.h"

using json = nlohmann::json;

namespace msg
{
class JsonMesasge: public BaseMessage
{
public:
    json msg;//这个成员变量对外暴露。
    JsonMesasge(message_type type)
    : BaseMessage(type)
    {
        // mylogd("type:%d", type);
    }
    ~JsonMesasge() override = default;
    void read(std::istream& stream) override
    {
        std::string s;
        readVal(stream, s);
        msg = json::parse(s);
    }
    uint32_t getSize()  override
    {
        return sizeof(uint32_t) + msg.dump().size();
    }
protected:
    void doserialize(std::ostream& stream)  override
    {
        // mylogd("%s", msg.dump().c_str());
        writeVal(stream, msg.dump());
    }
    template<typename T>
    T get(const std::string& what, const T& def) const
    {
        try {
            if(!msg.count(what)) {
                return def;
            }
            return msg[what].get<T>();
        } catch(...) {
            return def;
        }
    }
private:
};

} // namespace msg

