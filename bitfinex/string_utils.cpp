#include "string_utils.h"
#include "x_assert.h"
#include <string.h>

std::string FormatString(const char* fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    const std::string ret = FormatStringV(fmt, vl);
    va_end(vl);
    return ret;
}

std::string FormatStringV(const char* fmt, va_list vv)
{
    va_list v2;
    va_copy(v2, vv); // vv will be destroyed after the first call to vsnprintf()
    std::string ret(strlen(fmt)+32, 0);
    ssize_t needed = vsnprintf(&ret[0], ret.size(), fmt, vv);
    if(needed < 0){
        va_end(v2);
        return "Error calling vsnprintf("+std::string(fmt)+")";
    }
    else if(needed >= (ssize_t)ret.size()) {
        ret.resize(needed+1);
        needed = vsnprintf(&ret[0], ret.size(), fmt, v2);
        ASSERT_GT(needed, 0);
        ASSERT_LE(needed, (ssize_t)ret.size());
    }
    ret.resize(needed);
    va_end(v2);
    return ret;
}

std::string CommaNum(const int64_t val)
{
    char buf[32];
    char* pp = &buf[sizeof(buf)-1];
    *pp = 0;
    --pp;
    int dd = 0;
    int64_t vv = labs(val);
    do {
        if(dd == 3){
            dd = 0;
            *pp = ',';
            --pp;
        }
        *pp = '0' + (vv % 10);
        ++dd;
        --pp;
        vv /= 10;
    } while(vv);
    if(val < 0) {
        *pp = '-';
        --pp;
    }
    ++pp;
    ASSERT(pp >= buf);
    ASSERT_LT(strlen(pp), sizeof(buf));
    return pp;
}
