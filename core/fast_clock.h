#pragma once
#include "gcc_utils.h"
#include <chrono>
#include <stdint.h>
#include <x86intrin.h>

struct CpuClock // TODO: would be nice to support chrono casting
{
    using time_point = uint64_t; // TODO: class so we can ostream <<, converting to seconds/ms
    using duration   = int64_t;
    static FORCE_INLINE time_point now() { return __rdtsc(); } // 1ns

    static FORCE_INLINE time_point now2() // 9ns but avoids out-of-order exec
        {
        uint32_t aux;
        COMPILER_BARRIER();
        uint64_t res = __rdtscp(&aux);
        COMPILER_BARRIER();
        return res;
        }
};

using CpuTimeStamp = CpuClock::time_point;
using ClockCycles  = CpuClock::duration;


double ns_to_cc_factor();
double cc_to_ns_factor();

ClockCycles              ns_to_cc( std::chrono::nanoseconds);
std::chrono::nanoseconds cc_to_ns( ClockCycles);
