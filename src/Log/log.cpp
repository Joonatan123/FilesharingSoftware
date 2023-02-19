#define BOOST_LOG_DYN_LINK 1
#include "log.hpp"

bool Log::importantMessageDisplayCallbackSet = false;
boost::function<void(std::string)> Log::importantMessageDisplayCallback;
Log::src::severity_logger<Log::logging::trivial::severity_level> Log::lg;
// The operator is used for regular stream formatting
std::ostream &Log::operator<<(std::ostream &strm, Source level)
{
    static const char *strings[] =
        {
            "undefined",
            "file",
            "global",
            "network",
            "packet",
            "main_app",
            "p2p",
        };

    if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast<int>(level);

    return strm;
}

// The operator is used when putting the severity level to log
Log::logging::formatting_ostream &Log::operator<<(
    logging::formatting_ostream &strm,
    logging::to_log_manip<Source, tag::source> const &manip)
{
    static const char *strings[] =
        {
            "UNDEFINED",
            "FILE",
            "GLOBAL",
            "NETWORK",
            "PACKET",
            "MAIN_APP",
            "DOWNLOAD",
            "P2P",
        };

    Source level = manip.get();
    if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast<int>(level);

    return strm;
}

bool Log::filter_function(logging::value_ref<logging::trivial::severity_level, tag::severity> const &level,
                          logging::value_ref<Source, tag::source> const &sourceValue)
{
    return level != logging::trivial::severity_level::trace; // level >= boost::log::trivial::severity_level::trace || tag == Source::main_app;
}


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
void Log::packet_formatter(logging::record_view const &rec, logging::formatting_ostream &strm)
{
    // auto sourcea = rec[source];
    strm << "[packet]";
    strm << "[" << FormatTime(boost::posix_time::second_clock::universal_time()) << "]";
    strm << " : ";
    strm << rec[logging::expressions::smessage];
}

void Log::formatter(logging::record_view const &rec, logging::formatting_ostream &strm)
{
    std::wstring timeString = FormatTime(boost::posix_time::second_clock::universal_time());

    auto severity = rec[logging::trivial::severity];
    auto source_ = rec[source]; // name has a to avoid name conflict(cant compile)

    if (source_ == Source::packet)
    {
        packet_formatter(rec, strm);
        return;
    }

    strm << "[" << source_ << "]";
    strm << "[" << severity << "]";
    strm << "[" << timeString << "]";

    // Format the message here...
    strm << " : " << rec[logging::expressions::smessage];
}

void Log::coloring_formatter(
    logging::record_view const &rec, logging::formatting_ostream &strm)
{
    auto severity = rec[logging::trivial::severity];
    if (severity)
    {
        // Set the color
        switch (severity.get())
        {
        case logging::trivial::severity_level::debug:
            strm << "\033[32m";
            break;
        case logging::trivial::severity_level::info:
            if (rec[source] == Source::packet)
                strm << "\033[35m";
            break;
        case logging::trivial::severity_level::warning:
            strm << "\033[33m";
            break;
        case logging::trivial::severity_level::error:
        case logging::trivial::severity_level::fatal:
            strm << "\033[91m";
            break;
        default:
            break;
        }
    }

    formatter(rec, strm);

    if (severity)
    {
        // Restore the default color
        strm << "\033[0m";
    }
}

void Log::init()
{
    logging::add_common_attributes();
    typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;
    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();

    boost::shared_ptr<std::ostream> pStream(&std::clog, boost::null_deleter()); //, logging::empty_deleter()
    sink->locked_backend()->add_stream(pStream);

    sink->set_formatter(&coloring_formatter);

    // logging::trivial::severity != logging::trivial::severity_level::info
    // Source::global != source

    // expr::has_attr(severity)

    // sink->set_filter(logging::trivial::severity != logging::trivial::severity_level::trace);
    namespace phoenix = boost::phoenix;

    sink->set_filter(phoenix::bind(&filter_function, severity.or_none(), source.or_none()));

    logging::core::get()->add_sink(sink);
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
Log::LogStream::LogStream(logging::trivial::severity_level severity, Source source)
{
    this->severity = severity;
    this->source = source;
}
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
        temp << severity << " : " << stream.str();
        importantMessageDisplayCallback(temp.str());
    }
    if (!importantMessage || print.print)
    {
        BOOST_LOG_SCOPED_THREAD_TAG("Source", source);
        BOOST_LOG_SEV(lg, severity) << stream.str();
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

    stream << t;
    BOOST_LOG_SCOPED_THREAD_TAG("Source", packet);
    BOOST_LOG_SEV(lg, Log::logging::trivial::severity_level::info) << stream.str();
}

// #define LOG_SEV(severity) (Log::LogStream(Log::logging::trivial::severity_level::severity))

#define LOG_MANUAL(severity, source) (Log::LogStream(Log::logging::trivial::severity_level::severity, Log::Source::source))

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