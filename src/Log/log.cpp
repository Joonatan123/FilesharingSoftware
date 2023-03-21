#define BOOST_LOG_DYN_LINK 1
#include "log.hpp"
bool Log::useColorPrinting = true;
bool Log::importantMessageDisplayCallbackSet = false;
boost::function<void(std::string)> Log::importantMessageDisplayCallback;

void Log::SetMessageCallback(boost::function<void(std::string)> i)
{
    importantMessageDisplayCallbackSet = true;
    importantMessageDisplayCallback = i;
}
std::wstring Log::FormatTime(boost::posix_time::ptime now)
{
    using namespace boost::posix_time;
    static std::locale loc(std::wcout.getloc(),
                           new wtime_facet(L"%H:%M:%S")); //%Y%m%d

    std::basic_stringstream<wchar_t> wss;
    wss.imbue(loc);
    wss << now;
    return wss.str();
}
std::string Log::SourceToString(Log::Source source)
{
    static const std::string names[] = {
        "UNDEFINED",
        "FILE",
        "GLOBAL",
        "NETWORK",
        "PACKET",
        "MAIN_APP",
        "P2P",
    };
    return names[source];
}
std::string Log::SeverityToString(Log::SeverityLevel severity)
{
    static const std::string names[] = {
        "TRACE",
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "FATAL",
    };
    return names[severity];
}
void Log::SetColor(Log::SeverityLevel severity)
{

    // Set the color
    switch (severity)
    {
    case Log::SeverityLevel::debug:
        std::cerr << "\033[32m";
        break;
    case Log::SeverityLevel::warning:
        std::cerr << "\033[33m";
        break;
    case Log::SeverityLevel::error:
    case Log::SeverityLevel::fatal:
        std::cerr << "\033[91m";
        break;
    default:
        break;
    }
    // Restore the default color
    // strm << "\033[0m";
}

void Log::init()
{
}

Log::Important::Important(){};
Log::Important::Important(bool print) : print(print) {}

/*template <typename T>
std::stringstream &Log::LogStream::operator<<(T t)
{
    stream << t;
    return stream;
}*/
std::stringstream &Log::LogStream::operator<<(Important t)
{
    importantMessage = true;
    print = t;
    return stream;
}
/*Log::LogStream::LogStream(logging::trivial::severity_level severity, Source source)
{
    this->severity = severity;
    this->source = source;
}*/
/*Log::LogStream::LogStream(logging::trivial::severity_level severity)
{
    this->severity = severity;
    if (ScopedTag::tagMap.count(boost::this_thread::get_id()))
        this->source = ScopedTag::tagMap[boost::this_thread::get_id()].stack->top();
    else
        source = Source::undefined;
}*/

Log::LogStream::~LogStream()
{
    if (importantMessage && importantMessageDisplayCallbackSet)
    {
        std::stringstream temp("");
        temp << Log::SeverityToString(severity) << " : " << stream.str();
        importantMessageDisplayCallback(temp.str());
    }
    if (!importantMessage || print.print)
    {
        if (useColorPrinting)
            Log::SetColor(severity);
        std::cerr << '[' << Log::SourceToString(source) << "][" << Log::SeverityToString(severity) << "] : " << stream.str() << std::endl;
        if (useColorPrinting)
            std::cerr << "\033[0m";
    }
}

void Log::LogPacketStream::operator<<(std::string t)
{

    static const std::vector<std::string> excludedPackets =
        {
            "DOWNLOAD_ACK",
            "REDIRECT",
        };
    for (std::string packetName : excludedPackets)
    {
        if (packetName == t)
            return;
    }
    if (useColorPrinting)
        std::cerr << "\033[35m";
    std::cerr << "[PACKET] : Received packet of type: " << t << std::endl;
    if (useColorPrinting)
        std::cerr << "\033[0m";
}

// #define LOG_SEV(severity) (Log::LogStream(Log::logging::trivial::severity_level::severity))

//#define LOG_MANUAL(severity, source) (Log::LogStream(Log::logging::trivial::severity_level::severity, Log::Source::source))

/*#define LOG_TRACE LOG_SEV(trace)
#define LOG_DEBUG LOG_SEV(debug)
#define LOG_INFO LOG_SEV(info)
#define LOG_WARNING LOG_SEV(warning)
#define LOG_ERROR LOG_SEV(error)
#define LOG_FATAL LOG_SEV(fatal)*/

/*#define LOG_TRACE(source) LOG_MANUAL(trace, source)
#define LOG_DEBUG(source) LOG_MANUAL(debug, source)
#define LOG_INFO(source) LOG_MANUAL(info, source)
#define LOG_WARNING(source) LOG_MANUAL(warning, source)
#define LOG_ERROR(source) LOG_MANUAL(error, source)
#define LOG_FATAL(source) LOG_MANUAL(fatal, source)*/

// Log::Source::{source tag}
// #define LOG_ADD_THREAD_TAG(source) auto a = Log::ScopedTag::AddStackEntry(source)

//#define LOG_PACKET (Log::LogPacketStream())

/*int main(int, char *[])
{
    Log::init();

    using namespace Log::logging::trivial;

    // auto a = Log::ScopedTag::AddStackEntry(Log::Source::global);
    LOG_ADD_THREAD_TAG(Log::Source::global);
    LOG_WARNING << "warning";
    {
        LOG_ADD_THREAD_TAG(Log::Source::network);
        LOG_INFO << "asddsa INFO";
        LOG_MANUAL(Log::logging::trivial::severity_level::error, Log::Source::file) << "manual";
        //(Log::LogStream(logging::trivial::severity_level::trace));
    }
    // Log::ScopedTag::LogMessage(Log::logging::trivial::severity_level::fatal, "scope");

    return 0;
}*/