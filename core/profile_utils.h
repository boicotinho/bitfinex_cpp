#pragma once
#include "fast_clock.h"
#include <string>
#include <chrono>
#include <vector>
#include <stdint.h>
#include <x86intrin.h>

FORCE_INLINE CpuTimeStamp rdtscp() { return CpuClock::now2(); } // TODO: remove

// http://oliveryang.net/2015/09/pitfalls-of-TSC-usage/

std::string format_cc_timings_table(
        std::vector<CpuClock::time_point> const& a_cc_timings,
        std::string                       const& a_title);
