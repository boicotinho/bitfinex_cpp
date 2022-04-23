#pragma once
#include "core/gcc_utils.h"
#include <stdexcept>
#include <sstream>
#include <string>
#include <stdarg.h>
#include <errno.h>

std::string pretty_errno(int eno = errno);
std::string format_error(const char* file, int line, const char* func, int eno, const char* fmt, ...) __attribute__((format(printf, 5, 6)));

using Error = std::runtime_error;

#define THROW_ERROR(...) throw Error(format_error(__FILE__,__LINE__,__FUNCTION__, 0,     __VA_ARGS__))
#define THROW_ERRNO(...) throw Error(format_error(__FILE__,__LINE__,__FUNCTION__, errno, __VA_ARGS__))
#define THROW_CERR(EXPR) do \
        { \
            std::stringstream ss; \
            ss << EXPR; \
            THROW_ERROR("%s", ss.str().c_str()); \
        } while(0)

#define CHECK_SYSCALL(SYSCALL)  \
        [&](){  \
            const auto __chkscll_res_ = (SYSCALL); \
            if((intptr_t) __chkscll_res_ < 0) \
                THROW_ERRNO("System call failed: %s", #SYSCALL); \
            return __chkscll_res_; \
        }()
