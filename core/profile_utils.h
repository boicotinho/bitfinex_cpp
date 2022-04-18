#pragma once
#include "gcc_utils.h"
#include <string>
#include <chrono>
#include <vector>
#include <stdint.h>
#include <x86intrin.h>

using CpuTimeStamp = uint64_t;
using ClockCycles  = int64_t;

// http://oliveryang.net/2015/09/pitfalls-of-TSC-usage/
FORCE_INLINE CpuTimeStamp rdtscp()
{
    uint32_t aux;
    COMPILER_BARRIER();
    uint64_t res = __rdtscp(&aux);
    COMPILER_BARRIER();
    return res;
}

FORCE_INLINE CpuTimeStamp rdtsc_no_p()
{
    COMPILER_BARRIER();
    uint64_t res = __rdtsc();
    COMPILER_BARRIER();
    return res;
}

std::string format_cc_timings_table(
        std::vector<CpuTimeStamp> const& a_cc_timings,
        std::string               const& a_title);

double ns_to_cc_factor();
double cc_to_ns_factor();

ClockCycles              ns_to_cc( std::chrono::nanoseconds);
std::chrono::nanoseconds cc_to_ns( ClockCycles);
