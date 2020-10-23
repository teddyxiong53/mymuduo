#pragma once

#include <exception>
#include <string>
#include <cstring>

class SnapException : public std::exception
{
    char *m_text;
public:
    SnapException(const char *text)
    {
        m_text = new char[std::strlen(text)+1];
        std::strcpy(m_text, text);
    }
    SnapException(const std::string& text): SnapException(text.c_str())
    {

    }
    SnapException(const SnapException& other) : SnapException(other.what())
    {

    }
    ~SnapException() throw() override
    {
        delete[] m_text;
    }
    const char *what() const noexcept override
    {
        return m_text;
    }
};
