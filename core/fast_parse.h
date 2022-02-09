#pragma once
#include "gcc_utils.h"
#include <stdint.h>

// from_chars(,,float) only availabe from GCC 11 it seems.
#if (__cplusplus >= 201703L && __GNUC__ >= 11)
    #define FAST_PARSE_USE_STD_CHARCONV 1
    #include <charconv> // std::from_chars(,,float)
#else
    #include <cstdlib>  // std::strtof
#endif

//=================================================================================================
FORCE_INLINE void fast_parse(
    char const*&        pp,
    char const* const   end,
    uint64_t&           res)
{
    res = 0;
    for(; LIKELY(pp < end); ++pp)
    {
        uint8_t const digit = *pp - '0';
        if(digit > 9)
            return;
        res = res * 10 + digit;
    }
}

//=================================================================================================
FORCE_INLINE void fast_parse(
    char const*&        pp,
    char const* const   end,
    uint32_t&           res)
{
    uint64_t tmp;
    fast_parse(pp, end, tmp);
    res = tmp;
}

//=================================================================================================
FORCE_INLINE void fast_parse(
    char const*&        pp,
    char const* const   end,
    float&              res)
{
#ifdef FAST_PARSE_USE_STD_CHARCONV
    auto [ptr, ec] = std::from_chars(pp, end, res);
    pp = ptr;
#else
    if(LIKELY(pp < end))
    {
        char* parse_end = const_cast<char*>(end);
        res = std::strtof(pp, &parse_end);
        auto const parsed_len = parse_end - pp;
        pp += parsed_len;
    }
#endif
}
