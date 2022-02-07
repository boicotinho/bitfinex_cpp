#pragma once

#include "gcc_utils.h"
#include <string>
#include <vector>
#include <x86intrin.h>
#include <stdint.h>

using CpuTimeStamp = uint64_t;

FORCE_INLINE CpuTimeStamp rdtscp()
{
    uint32_t aux;
    COMPILER_BARRIER();
    uint64_t res = __rdtscp(&aux);
    COMPILER_BARRIER();
    return res;
}

// Returns a string with timings table.
// 
// Argument a_cc_timings is a vector of timestamp diffs captured with rdtscp().
// E.g.
//
//  vector<CpuTimeStamp> ts;
//  for(int ii = 0; ii < 1000; ii++) {
//      auto t0 = rdtscp();
//      my_code();
//      auto t1 = rdtscp();
//      ts.push_back(t1 - t0);
//  }
//  cout << FormatCcTimingsTable(ts, "MyCode");
//
//  output (cc stands for CPU clock cycles):
//
//  Perf 1,000 x MyCode
//      0.0 % :          162 cc 
//     10.0 % :          170 cc 
//     50.0 % :          196 cc 
//     75.0 % :          204 cc 
//     90.0 % :          212 cc 
//     99.0 % :          236 cc 
//     99.5 % :          242 cc 
//     99.8 % :          260 cc 
//     99.9 % :          262 cc 
//    100.0 % :       12,828 cc 
//    average :          193 cc 
std::string FormatCcTimingsTable(std::vector<CpuTimeStamp>& a_cc_timings, const std::string& a_title);
