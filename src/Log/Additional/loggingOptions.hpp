#include "Source.hpp"

namespace Log
{
    bool filter_function(logging::value_ref<logging::trivial::severity_level, tag::severity> const &level,
                   logging::value_ref<Source, tag::source> const &sourceValue);
}