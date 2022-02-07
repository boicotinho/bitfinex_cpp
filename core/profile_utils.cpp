#include "profile_utils.h"
#include "string_utils.h"
#include <numeric>
#include <algorithm>
#include <sstream>

std::string FormatCcTimingsTable(std::vector<CpuTimeStamp>& perf, const std::string& title)
{
    std::stringstream ss;
    std::sort(perf.begin(), perf.end());
    ss << "\nPerf " << CommaNum(perf.size()) << " x " << title << "\n";
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
        ss << FormatString("  %5.1f %% : %12s cc\n", qq * 100, CommaNum(qtl(qq)).c_str());
    const uint64_t sum = std::accumulate(perf.begin(), perf.end(), 0uLL);
    const uint64_t avg = sum/perf.size();
    const double   favg = ((double)sum) / (double)perf.size();
    ss << FormatString("  average : %12s cc\n", CommaNum(avg).c_str());
    return ss.str();
}
