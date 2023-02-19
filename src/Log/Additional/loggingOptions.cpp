#include "loggingOptions.hpp"
bool Log::filter_function(logging::value_ref<logging::trivial::severity_level, tag::severity> const &level,
                          logging::value_ref<Source, tag::source> const &sourceValue)
{
    return level != logging::trivial::severity_level::trace; // level >= boost::log::trivial::severity_level::trace || tag == Source::main_app;
}
