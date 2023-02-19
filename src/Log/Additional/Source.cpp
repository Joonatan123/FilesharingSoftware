#include "Source.hpp"

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
