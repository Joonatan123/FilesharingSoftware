#include <ostream>
#include <fstream>
#include <sstream>
#include <iostream>

#include <boost/function.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>

#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/phoenix/bind.hpp>

#include <boost/log/support/date_time.hpp>

#pragma once

namespace Log
{
    enum Source
    {
        undefined,
        file,
        global,
        network,
        packet,
        main_app,
        p2p,
    };

    std::string SourceToString(Source source);

    enum SeverityLevel
    {
        trace,
        debug,
        info,
        warning,
        error,
        fatal,
    };

    std::string SeverityToString(SeverityLevel severity);

}

namespace Log
{
    extern bool useColorPrinting;
    extern bool importantMessageDisplayCallbackSet;
    extern boost::function<void(std::string)> importantMessageDisplayCallback;
    void SetMessageCallback(boost::function<void(std::string)> i);
    std::wstring FormatTime(boost::posix_time::ptime now);
    void SetColor(Log::SeverityLevel severity);
    void init();
    class Important
    {

    public:
        bool print = false;
        Important();
        Important(bool print);
    };
    class LogStream
    {
    public:
        std::stringstream stream;
        SeverityLevel severity;
        Source source;
        bool importantMessage = false;
        Important print;
        /*template <typename T>
        std::stringstream &operator<<(T t);*/
        template <typename T>
        std::stringstream &operator<<(T t)
        {
            stream << t;
            return stream;
        }
        std::stringstream &operator<<(Important t);
        LogStream(SeverityLevel severity, Source source)
        {
            this->severity = severity;
            this->source = source;
        }
        // LogStream(logging::trivial::severity_level severity);

        ~LogStream();
    };
    class LogPacketStream
    {
    public:
        std::stringstream stream;
        void operator<<(std::string t);
    };
}
// #define LOG_SEV(severity) (Log::LogStream(Log::logging::trivial::severity_level::severity))

#define LOG_MANUAL(severity, source) (Log::LogStream(Log::SeverityLevel::severity, Log::Source::source))

/*#define LOG_TRACE LOG_SEV(trace)
#define LOG_DEBUG LOG_SEV(debug)
#define LOG_INFO LOG_SEV(info)
#define LOG_WARNING LOG_SEV(warning)
#define LOG_ERROR LOG_SEV(error)
#define LOG_FATAL LOG_SEV(fatal)*/

#define LOG_TRACE(source) LOG_MANUAL(trace, source)
#define LOG_DEBUG(source) LOG_MANUAL(debug, source)
#define LOG_INFO(source) LOG_MANUAL(info, source)
#define LOG_WARNING(source) LOG_MANUAL(warning, source)
#define LOG_ERROR(source) LOG_MANUAL(error, source)
#define LOG_FATAL(source) LOG_MANUAL(fatal, source)

// Log::Source::{source tag}
// #define LOG_ADD_THREAD_TAG(source) auto a = Log::ScopedTag::AddStackEntry(source)

#define LOG_PACKET (Log::LogPacketStream())

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
