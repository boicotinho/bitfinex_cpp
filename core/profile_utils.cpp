#include "profile_utils.h"
#include "time_util.h"
#include "string_utils.h"
#include <vector>
#include <numeric>
#include <thread>
#include <algorithm>
#include <sstream>

std::string format_cc_timings_table( std::vector<CpuTimeStamp> const& a_perf
                                   , std::string               const& a_title
                                   )
{
    std::stringstream ss;
    auto perf = a_perf;
    std::sort(perf.begin(), perf.end());
    const double cc2us = cc_to_ns_factor() / 1000.;
    ss << "\nPerf " << CommaNum(perf.size()) << " x " << a_title << "\n";
    if(perf.empty())
        return ss.str();
    auto qtl = [&](double x) -> CpuTimeStamp
    {
        size_t ix = (double)perf.size() * x;
        if(ix>= perf.size())
            ix = perf.size()-1;
        return perf[ix];
    };
    for(double qq : {0.0, 0.10, 0.5, 0.75, 0.9, 0.99, 0.995, 0.998, 0.999, 1.0} )
        ss << FormatString("  %5.1f %% : %12s cc  %7.3f us\n",
                        qq * 100, CommaNum(qtl(qq)).c_str(), qtl(qq) * cc2us);
    const uint64_t sum = std::accumulate(perf.begin(), perf.end(), 0uLL);
    const uint64_t avg = sum/perf.size();
    const double   favg = ((double)sum) / (double)perf.size();
    ss << FormatString("  average : %12s cc  %7.3f us\n",
            CommaNum(avg).c_str(), favg * cc2us);
    return ss.str();
}

struct TscPair
{
    using Clock = std::chrono::high_resolution_clock;
    CpuTimeStamp        cc {};
    Clock::time_point   ns {};
    static TscPair now()
        {
        TscPair res;
        res.cc = rdtscp();
        res.ns = Clock::now();
        COMPILER_BARRIER();
        return res;
        }
};

static TscPair g_t0 = TscPair::now();
double calc_cpu_freq_hz(std::chrono::nanoseconds const a_min_time_span = 100_ms)
{
    if(!g_t0.cc)
        g_t0 = TscPair::now();
    TscPair t1 = TscPair::now();
    auto const elapsed = t1.ns - g_t0.ns;
    if(elapsed < a_min_time_span)
    {
        std::this_thread::sleep_for(a_min_time_span - elapsed);
        t1 = TscPair::now();
    }
    std::chrono::duration<double> secs (t1.ns - g_t0.ns);
    double const freq_hz = double(t1.cc - g_t0.cc) / secs.count();
    return freq_hz;
}

// http://oliveryang.net/2015/09/pitfalls-of-TSC-usage/
double ns_to_cc_factor()
{
    static const double g_ns_2_cc = calc_cpu_freq_hz() / 1e9;
    return g_ns_2_cc;
}

double cc_to_ns_factor()
{
    static const double g_cc_2_ns = 1. / ns_to_cc_factor();
    return g_cc_2_ns;
}

ClockCycles ns_to_cc(std::chrono::nanoseconds const a_ns)
{
    const double ns_2_cc = ns_to_cc_factor();
    const int64_t cc_duration = a_ns.count() * ns_2_cc;
    return cc_duration;
}

std::chrono::nanoseconds cc_to_ns(ClockCycles const a_cc)
{
    const double cc_2_ns = cc_to_ns_factor();
    const int64_t ns_duration = a_cc * cc_2_ns;
    return std::chrono::nanoseconds(ns_duration);
}
