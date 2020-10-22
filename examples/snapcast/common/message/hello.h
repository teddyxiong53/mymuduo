#pragma once
#include "json_message.h"
#include "common/utils.h"

namespace msg
{
class Hello: public JsonMesasge
{
public:
    Hello(): JsonMesasge(message_type::kHello)
    {

    }
    Hello(const std::string& macAddres,
        const std::string& id,
        size_t instance
    ) : JsonMesasge(message_type::kHello)
    {
        msg["MAC"] = macAddres;
        msg["HostName"] = ::getHostName();
        msg["Version"] = "1.0";
        msg["OS"] = ::getOS();
        msg["Arch"] = ::getArch();
        msg["Instance"] = instance;
        msg["ID"] = id;
        msg["SnapStreamProtocolVersion"] = 2;
    }
    ~Hello() override = default;

    std::string getMacAddress() const
    {
        return msg["MAC"];
    }

    std::string getHostName() const
    {
        return msg["HostName"];
    }

    std::string getVersion() const
    {
        return msg["Version"];
    }

    std::string getClientName() const
    {
        return msg["ClientName"];
    }

    std::string getOS() const
    {
        return msg["OS"];
    }

    std::string getArch() const
    {
        return msg["Arch"];
    }

    int getInstance() const
    {
        return get("Instance", 1);
    }

    int getProtocolVersion() const
    {
        return get("SnapStreamProtocolVersion", 1);
    }

    std::string getId() const
    {
        return get("ID", getMacAddress());
    }

    std::string getUniqueId() const
    {
        std::string id = getId();
        int instance = getInstance();
        if (instance != 1)
        {
            id = id + "#" + cpt::to_string(instance);
        }
        return id;
    }
private:
};


} // namespace msg

