#include <ostream>

#include <boost/core/null_deleter.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/value_extraction.hpp>

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

    // Define the attribute keywords
    namespace logging = boost::log;
    namespace src = boost::log::sources;
    namespace expr = boost::log::expressions;
    namespace sinks = boost::log::sinks;

    src::severity_logger<logging::trivial::severity_level> lg;

    // The operator is used for regular stream formatting
    std::ostream &operator<<(std::ostream &strm, Source level);
    BOOST_LOG_ATTRIBUTE_KEYWORD(source, "Source", Source)
    BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", logging::trivial::severity_level)

    // The operator is used when putting the severity level to log
    logging::formatting_ostream &operator<<(
        logging::formatting_ostream &strm,
        logging::to_log_manip<Source, tag::source> const &manip);
}