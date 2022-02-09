#pragma once
#include "gcc_utils.h"
#include <stdint.h>

#if (__cplusplus >= 201703L)
    #include <charconv>
#else
    #include <cstdlib>
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
#if (__cplusplus >= 201703L)
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
