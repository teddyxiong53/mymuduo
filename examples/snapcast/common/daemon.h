#pragma once

#include <string>

class Daemon
{
public:
    Daemon(const std::string& user, const std::string& group, const std::string& pidFile);
    virtual ~Daemon();
    bool daemonize();

private:
    int m_pidFileHandle;
    std::string m_user;
    std::string m_group;
    std::string m_pidFile;
};


