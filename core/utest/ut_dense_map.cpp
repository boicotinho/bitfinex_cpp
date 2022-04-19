#include <boost/test/unit_test.hpp>
#include "core/profile_utils.h"
#include "core/dense_map.h"
#include "core/hashers.h"
#include <unordered_map>

inline boost::test_tools::assertion_result exec_microbench(boost::unit_test::test_unit_id)
{
    #ifdef NDEBUG
        return true;
    #else
        return false;
    #endif
}

BOOST_AUTO_TEST_SUITE(core)

// For Profiling
constexpr int NN = 1000;
constexpr int QQ = 988;
constexpr int SS = 10248;

std::vector<int> const g_random_keys = []()
{
    std::vector<int> ret;
    for(int ii = 0; ii < QQ; ++ii)
    {
        const int kk = std::rand();
        ret.push_back(kk);
    }
    return ret;
}();

size_t GenKey(size_t xx)
{
    return g_random_keys[xx % g_random_keys.size()];
}

#if (DENSE_MAP_SUPPORTED)

BOOST_AUTO_TEST_CASE(dense_map)
{
    #if !(DENSE_MAP_SUPPORTED)
        BOOST_WARN("DENSE_MAP_SUPPORTED disabled. Skipping test");
        return;
    #endif

    struct TagToBookMapTraits
    {
        using Key               = uint32_t;
        using Value             = uint32_t;
        using Hasher            = IdentityHasher<Key>;
        enum { EMPTY_KEY        = 0 };
        enum { MIN_NUM_BUCKETS  = 32  };
    };

    DenseMap<TagToBookMapTraits> map;

    map.emplace(11, 111);
    map.emplace(22, 222);
    map.emplace(33, 333);

    auto pval = map.lookup_ptr(22);
    BOOST_REQUIRE(pval);
    BOOST_CHECK_EQUAL(*pval, 222);

    pval = map.lookup_ptr(11);
    BOOST_REQUIRE(pval);
    BOOST_CHECK_EQUAL(*pval, 111);

    //pval = map.lookup_ptr(33);
    //BOOST_REQUIRE(pval);
    //BOOST_CHECK_EQUAL(*pval, 333);

    //ap.erase(22);
    //uto pval = map.lookup_ptr(22);
    //OOST_CHECK(!pval);

    pval = map.lookup_ptr(11);
    BOOST_REQUIRE(pval);
    BOOST_CHECK_EQUAL(*pval, 111);

    //pval = map.lookup_ptr(33);
    //BOOST_REQUIRE(pval);
   // BOOST_CHECK_EQUAL(*pval, 333);
}

BOOST_AUTO_TEST_CASE(profile_dense_map, *boost::unit_test::precondition(exec_microbench))
{
    using ValType = uint16_t;
    using KeyType = uint16_t;

    struct TagToBookMapTraits
    {
        using Key               = KeyType;
        using Value             = ValType;
        using Hasher            = IdentityHasher<Key>;
        enum { EMPTY_KEY        = 0 };
        enum { MIN_NUM_BUCKETS  = SS };
    };

    DenseMap<TagToBookMapTraits> map;

    for(int qq = 0; qq < QQ; ++qq)
    {
        auto const kk = GenKey(qq);
        map.emplace(kk, qq);
    }

    std::vector<CpuTimeStamp> timings;
    timings.reserve(NN);

    for(int ii = 0; ii < NN; ++ii)
    {
        auto const kk = GenKey(ii);
        ValType const* vv;
        auto const t0 = rdtscp();
        vv = map.lookup_ptr(kk);
        auto const t1 = rdtscp();
        volatile auto dummy = *vv;
        timings.push_back(t1 - t0);
    }

    std::cout << format_cc_timings_table(timings, "DenseMap lookup hit");
    // Perf 1,000 x DenseMap lookup hit
    //     0.0 % :           50 cc
    //    10.0 % :           52 cc
    //    50.0 % :           58 cc
    //    75.0 % :           64 cc
    //    90.0 % :           72 cc
    //    99.0 % :           76 cc
    //    99.5 % :           78 cc
    //    99.8 % :          102 cc
    //    99.9 % :          110 cc
    //   100.0 % :          110 cc
    //   average :           59 cc
}

#endif // DENSE_MAP_SUPPORTED

BOOST_AUTO_TEST_CASE(profile_unordered_map, *boost::unit_test::precondition(exec_microbench))
{
    using ValType = uint16_t;
    using KeyType = uint16_t;

    std::unordered_map<KeyType,ValType> map;
    map.reserve(QQ);

    for(int qq = 0; qq < QQ; ++qq)
    {
        auto const kk = GenKey(qq);
        map.emplace(kk, qq);
    }

    std::cout << "size:    " << map.size() << "\n";
    std::cout << "buckets: " << map.bucket_count() << "\n";
    std::cout << "ld_fact: " << map.load_factor() << "\n";
    std::cout << "\n";

    std::vector<CpuTimeStamp> timings;
    timings.reserve(NN);

    for(int ii = 0; ii < NN; ++ii)
    {
        auto const kk = GenKey(ii);;
        ValType const* vv;
        auto const t0 = rdtscp();
        auto it = map.find(kk);
        vv = &it->second;
        auto const t1 = rdtscp();
        volatile auto dummy = *vv;
        timings.push_back(t1 - t0);
    }

    std::cout << format_cc_timings_table(timings, "std::unordered_map lookup hit");
    // Perf 1,000 x std::unordered_map lookup hit
    //     0.0 % :           68 cc
    //    10.0 % :           70 cc
    //    50.0 % :           82 cc
    //    75.0 % :          134 cc
    //    90.0 % :          154 cc
    //    99.0 % :          222 cc
    //    99.5 % :          234 cc
    //    99.8 % :          242 cc
    //    99.9 % :          288 cc
    //   100.0 % :          288 cc
    //   average :          102 cc
}

BOOST_AUTO_TEST_SUITE_END()
