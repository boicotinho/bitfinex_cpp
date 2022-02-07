#include <boost/test/unit_test.hpp>
#include <boost/container/flat_map.hpp>
#include <functional>
#include <map>

#include "test_utils.h"
#include "bitfinex/profile_utils.h"

namespace utf = boost::unit_test;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_SUITE(bitfinex)

// Tests that the book map interates in the correct order, 
// and that the contents are always sorted during iteration.

BOOST_AUTO_TEST_CASE(book_map_bid, * utf::tolerance(0.00001f))
{
    using ordered_map_t = boost::container::flat_map<uint32_t, float, std::less<uint32_t>>;

    ordered_map_t map;
    map.reserve(10);
    map.emplace( 32, 3.2f );
    map.emplace( 11, 1.1f );
    map.emplace( 44, 0.44f );
    uint32_t last_val = 0;
    for(auto ii : map)
    {
        BOOST_CHECK_LT(last_val, ii.first);
        last_val = ii.first;
    }
    BOOST_CHECK_EQUAL(map[32], 3.2f);
    BOOST_CHECK_EQUAL(map[11], 1.1f);
    BOOST_CHECK_EQUAL(map[44], 0.44f);
}

BOOST_AUTO_TEST_CASE(book_map_ask, * utf::tolerance(0.00001f))
{
    using ordered_map_t = boost::container::flat_map<uint32_t, float, std::greater<uint32_t>>;

    ordered_map_t map;
    map.reserve(10);
    map.emplace( 32, 3.2f );
    map.emplace( 11, 1.1f );
    map.emplace( 44, 0.44f );
    uint32_t last_val = 1000000;
    for(auto ii : map)
    {
        BOOST_CHECK_GT(last_val, ii.first);
        last_val = ii.first;
    }
    BOOST_CHECK_EQUAL(map[32], 3.2f);
    BOOST_CHECK_EQUAL(map[11], 1.1f);
    BOOST_CHECK_EQUAL(map[44], 0.44f);
    
    map.erase(32);
    BOOST_CHECK_EQUAL(map.count(32), 0);
    BOOST_CHECK_EQUAL(map[11], 1.1f);
    BOOST_CHECK_EQUAL(map[44], 0.44f);
}

AUTO_TEST_MICROBENCH(book_map_perf)
{
    using ordered_map_t = boost::container::flat_map<uint32_t, float, std::less<uint32_t>>;
    using key_t = ordered_map_t::key_type;
    using val_t = ordered_map_t::value_type::second_type;

    const int NN = 1000000;

    ordered_map_t map;
    map.reserve(NN);

    std::vector<CpuTimeStamp> timings;
    timings.reserve(NN);

    for(int ii = 0; ii < NN; ++ii)
    {
        key_t const key = std::hash<key_t>()(ii);
        val_t const val = ii;
        auto const t0 = rdtscp();
        map.emplace( key, val );
        auto const t1 = rdtscp();
        timings.push_back(t1 - t0);
    }

    std::cout << FormatCcTimingsTable(timings, "flat_map insert random");
    //  Perf 1,000,000 x flat_map insert random
    //      0.0 % :           52 cc
    //     10.0 % :          128 cc
    //     50.0 % :          136 cc
    //     75.0 % :          136 cc
    //     90.0 % :          144 cc
    //     99.0 % :          200 cc
    //     99.5 % :          328 cc
    //     99.8 % :          396 cc
    //     99.9 % :          828 cc
    //    100.0 % :      923,284 cc
    //    average :          158 cc    
}

AUTO_TEST_MICROBENCH(std_map_perf)
{
    using ordered_map_t = std::map<uint32_t, float, std::less<uint32_t>>;
    using key_t = ordered_map_t::key_type;
    using val_t = ordered_map_t::value_type::second_type;

    const int NN = 1000000;

    ordered_map_t map;

    std::vector<CpuTimeStamp> timings;
    timings.reserve(NN);

    for(int ii = 0; ii < NN; ++ii)
    {
        key_t const key = std::hash<key_t>()(ii);
        val_t const val = ii;
        auto const t0 = rdtscp();
        map.emplace( key, val );
        auto const t1 = rdtscp();
        timings.push_back(t1 - t0);
    }

    std::cout << FormatCcTimingsTable(timings, "std::map insert random");
    //  Perf 1,000,000 x std::map insert random
    //      0.0 % :          164 cc
    //     10.0 % :          624 cc
    //     50.0 % :          861 cc
    //     75.0 % :          920 cc
    //     90.0 % :        1,044 cc
    //     99.0 % :        6,908 cc
    //     99.5 % :       10,420 cc
    //     99.8 % :       28,509 cc
    //     99.9 % :       36,208 cc
    //    100.0 % :      526,605 cc
    //    average :        1,055 cc
}

BOOST_AUTO_TEST_SUITE_END()
