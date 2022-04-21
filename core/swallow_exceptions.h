#pragma once
#include "log.h"
#include <stdexcept>

// Some functions must be marked as noexcept
// because they are called from destructors.

#define SWALLOW_EXCEPTIONS(block)                                           \
    try                                                                     \
    {                                                                       \
        block;                                                              \
    }                                                                       \
    catch(const std::exception& ex)                                         \
    {                                                                       \
        TLOG_ERR("Exception swollen during stack unwinding: %s", ex.what());\
    }                                                                       \
    catch(...)                                                              \
    {                                                                       \
        TLOG_ERR("%s", "Unknown exception swollen during stack unwinding"); \
    }
