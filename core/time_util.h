#pragma once
#include <chrono>
#include <iosfwd>

using Nanos  = std::chrono::nanoseconds; // Convenient short name
using Micros = std::chrono::microseconds;
using Millis = std::chrono::milliseconds;

constexpr inline std::chrono::seconds      operator "" _s(unsigned long long tt)    {return std::chrono::seconds(tt);}
constexpr inline std::chrono::milliseconds operator "" _ms(unsigned long long tt)   {return std::chrono::milliseconds(tt);}
constexpr inline std::chrono::microseconds operator "" _us(unsigned long long tt)   {return std::chrono::microseconds(tt);}
constexpr inline std::chrono::nanoseconds  operator "" _ns(unsigned long long tt)   {return std::chrono::nanoseconds(tt);}
constexpr inline std::chrono::seconds      operator "" _mins(unsigned long long tt) {return std::chrono::seconds(tt)*60;}
constexpr inline std::chrono::seconds      operator "" _hours(unsigned long long tt){return std::chrono::seconds(tt)*60*60;}
constexpr inline std::chrono::seconds      operator "" _days(unsigned long long tt) {return std::chrono::seconds(tt)*60*60*24;}

namespace std
{
    std::ostream& operator << (std::ostream& out, const std::chrono::nanoseconds      & tt);
    std::ostream& operator << (std::ostream& out, const std::chrono::microseconds     & tt);
    std::ostream& operator << (std::ostream& out, const std::chrono::milliseconds     & tt);
    std::ostream& operator << (std::ostream& out, const std::chrono::seconds          & tt);
    std::ostream& operator << (std::ostream& out, const std::chrono::minutes          & tt);
    std::ostream& operator << (std::ostream& out, const std::chrono::hours            & tt);
    std::ostream& operator << (std::ostream& out, const std::chrono::duration<double> & tt);
}

constexpr auto INFINITE = 365_days;

constexpr std::chrono::microseconds chrono_from_timeval(const timeval& tv)
{
    using namespace std::chrono;
    return seconds(tv.tv_sec) + microseconds(tv.tv_usec);
}

inline std::chrono::microseconds chrono_from_timeval(const timeval* tt)
{
    return tt ? chrono_from_timeval(tt) : INFINITE;
}

constexpr std::chrono::nanoseconds chrono_from_timespec(const timespec& tt)
{
    return std::chrono::seconds(tt.tv_sec) + std::chrono::nanoseconds(tt.tv_nsec);
}

inline std::chrono::nanoseconds chrono_from_timespec(const timespec* tt)
{
    return tt ? chrono_from_timespec(tt) : INFINITE;
}

template<typename Rep, typename Period>
inline timeval timeval_from_chrono(std::chrono::duration<Rep,Period> tsp)
{
    using namespace std::chrono;
    auto const us = duration_cast<microseconds>(tsp);
    auto const secs = duration_cast<seconds>(tsp);
    return { secs.count(), (us % seconds(1)).count() };
}

template<typename Rep, typename Period>
inline timespec timespec_from_chrono(std::chrono::duration<Rep,Period> tsp)
{
    using namespace std::chrono;
    auto const ns = duration_cast<nanoseconds>(tsp);
    auto const secs = duration_cast<seconds>(tsp);
    return { secs.count(), (ns % seconds(1)).count() };
}
