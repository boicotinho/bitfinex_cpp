#include <boost/test/unit_test.hpp>
#include "core/profile_utils.h"
#include "core/dense_map.h"
#include "core/hashers.h"

#if defined(__AVX2__)

BOOST_AUTO_TEST_SUITE(core)

BOOST_AUTO_TEST_CASE(dense_map)
{
    #if !defined(__AVX2__)
        BOOST_WARN("AVX2 not supported. Skipping test");
        return;
    #endif

    struct TagToBookMapTraits
    {
        using Key               = uint32_t;
        using Hasher            = IdentityHasher<Key>;
        enum { EMPTY_KEY        = 0 };
        enum { MIN_NUM_BUCKETS  = 32  };
        using Value             = uint32_t;
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

    pval = map.lookup_ptr(33);
    BOOST_REQUIRE(pval);
    BOOST_CHECK_EQUAL(*pval, 333);

    //ap.erase(22);
    //uto pval = map.lookup_ptr(22);
    //OOST_CHECK(!pval);

    pval = map.lookup_ptr(11);
    BOOST_REQUIRE(pval);
    BOOST_CHECK_EQUAL(*pval, 111);

    pval = map.lookup_ptr(33);
    BOOST_REQUIRE(pval);
    BOOST_CHECK_EQUAL(*pval, 333);
}

BOOST_AUTO_TEST_SUITE_END()

#endif // __AVX2__