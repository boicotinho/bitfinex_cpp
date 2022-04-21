#include "profile_utils.h"
#include "time_util.h"
#include "string_utils.h"
#include <vector>
#include <numeric>
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
