
#include <sstream>
#include <string>

export module Log;

// implement a quite acceptable error system

export class Logger
{
    std::stringstream stream;

  public:
    Logger() = default;

    template <typename... Args> void Log(Args... args)
    {
        (stream << ... << args);
    }

    std::string dump()
    {
        return stream.str(); 
    }
};

static inline Logger  logged;

export Logger &GetSingletonLogger()
{
    return logged;
}
