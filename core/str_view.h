#pragma once
#include <stddef.h>

#if(__cplusplus >= 201703L)
    #include <string_view>
    using StrView = std::string_view;
#else
    #include <boost/utility/string_view.hpp>
    using StrView = boost::string_view;
#endif
