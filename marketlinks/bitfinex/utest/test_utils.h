#pragma once

// This file could be made a precompiled header to speed up test builds.

#include <boost/test/unit_test.hpp>

inline boost::test_tools::assertion_result exec_microbench(boost::unit_test::test_unit_id)
{
    #ifdef NDEBUG
        return true;
    #else
        return false;
    #endif
}

#define AUTO_TEST_MICROBENCH(name) \
    BOOST_AUTO_TEST_CASE(name, *boost::unit_test::precondition(exec_microbench))
