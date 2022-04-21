#pragma once
#include "fast_clock.h"
#include <string>
#include <chrono>
#include <vector>
#include <stdint.h>
#include <x86intrin.h>

FORCE_INLINE CpuTimeStamp rdtscp()
{
    uint32_t aux;
    COMPILER_BARRIER();
    uint64_t res = __rdtscp(&aux);
    COMPILER_BARRIER();
    return res;
}

// http://oliveryang.net/2015/09/pitfalls-of-TSC-usage/

std::string format_cc_timings_table(
        std::vector<CpuTimeStamp> const& a_cc_timings,
        std::string               const& a_title);
