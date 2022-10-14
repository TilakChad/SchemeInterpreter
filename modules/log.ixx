module;

#include <sstream>
#include <string>

#include <iostream>
#include <typeinfo>


export module Log;

// implement a quite acceptable error system

export class Logger
{
    std::stringstream stream;

  public:
    Logger()
    {
        stream = std::stringstream();
    }

    template <typename... Args> void Log(const Args &...args)
    {
        (stream << ... << args);
    }

    std::string dump()
    {
        return stream.str();
    }
};

static Logger  logged;

export Logger &GetSingletonLogger()
{
    return logged;
}
