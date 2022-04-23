#pragma once

#if(__cplusplus >= 201703L)
    #include <any>
    using any = std::any;
#else
    #include <boost/any.hpp>
    using Any = boost::any;
#endif
