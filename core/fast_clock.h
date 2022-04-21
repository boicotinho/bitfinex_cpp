#pragma once
#include "gcc_utils.h"
#include <chrono>
#include <stdint.h>
#include <x86intrin.h>

using CpuTimeStamp = uint64_t;
using ClockCycles  = int64_t;

FORCE_INLINE CpuTimeStamp fast_now() { return __rdtsc(); }

double ns_to_cc_factor();
double cc_to_ns_factor();

ClockCycles              ns_to_cc( std::chrono::nanoseconds);
std::chrono::nanoseconds cc_to_ns( ClockCycles);
