#include "test_utils.h"
#include "bitfinex/order_book_p.h"
#include "core/profile_utils.h"


BOOST_AUTO_TEST_SUITE(bitfinex)

BOOST_AUTO_TEST_CASE(empty_book)
{
    using namespace bitfinex;
    using level_based::OrderBookP;
    using level_based::PxQx;
    using level_based::TOB;

    OrderBookP book;
    TOB tob;

    tob = book.get_tob(0);
    BOOST_CHECK(!tob.has_side(eSide::BID));
    BOOST_CHECK(!tob.has_side(eSide::ASK));
    BOOST_CHECK(tob.empty());

    tob = book.get_tob(-1);
    BOOST_CHECK(!tob.has_side(eSide::BID));
    BOOST_CHECK(!tob.has_side(eSide::ASK));
    BOOST_CHECK(tob.empty());

    for(int ii = 0; ii < 1000; ++ii)
    {
        tob = book.get_tob(ii);
        BOOST_CHECK(tob.empty());
    }

    book.erase_level(0, eSide::ASK);
    BOOST_CHECK(tob.empty());

    book.erase_level(0, eSide::BID);
    BOOST_CHECK(tob.empty());
}

BOOST_AUTO_TEST_CASE(simple_side)
{
    using namespace bitfinex;
    using level_based::OrderBookSideP;
    using level_based::PxQx;

    OrderBookSideP hbook;
    PxQx tob;

    tob = hbook.get_best_px(0, eSide::BID);
    BOOST_CHECK(tob.empty());

    // Insert order (41000, 0.0053f), result should be:
    // TOB[0] = (41000, 0.0053f)
    hbook.assign_level(41000, 0.0053f);
    tob = hbook.get_best_px(0, eSide::BID);
    BOOST_CHECK(!tob.empty());
    BOOST_CHECK_EQUAL(tob.price_level, 41000);
    BOOST_CHECK_EQUAL(tob.total_qty, 0.0053f);
    BOOST_CHECK(hbook.get_best_px(1, eSide::BID).empty());
    BOOST_CHECK(hbook.get_best_px(2, eSide::BID).empty());

    // Insert order (42000, 0.0042f), result should be:
    // TOB[0] = (42000, 0.0042f) (best price)
    // TOB[1] = (41000, 0.0053f)
    hbook.assign_level(42000, 0.0042f);
    tob = hbook.get_best_px(0, eSide::BID);
    BOOST_CHECK_EQUAL(tob.price_level, 42000);
    BOOST_CHECK_EQUAL(tob.total_qty, 0.0042f);
    tob = hbook.get_best_px(1, eSide::BID);
    BOOST_CHECK_EQUAL(tob.price_level, 41000);
    BOOST_CHECK_EQUAL(tob.total_qty, 0.0053f);

    // Insert order (40000, 0.0011f), result should be:
    // TOB[0] = (42000, 0.0042f) (best price)
    // TOB[1] = (41000, 0.0053f)
    // TOB[2] = (40000, 0.0011f)
    hbook.assign_level(40000, 0.0011f);
    tob = hbook.get_best_px(0, eSide::BID);
    BOOST_CHECK_EQUAL(tob.price_level, 42000);
    BOOST_CHECK_EQUAL(tob.total_qty, 0.0042f);
    tob = hbook.get_best_px(1, eSide::BID);
    BOOST_CHECK_EQUAL(tob.price_level, 41000);
    BOOST_CHECK_EQUAL(tob.total_qty, 0.0053f);
    tob = hbook.get_best_px(2, eSide::BID);
    BOOST_CHECK_EQUAL(tob.price_level, 40000);
    BOOST_CHECK_EQUAL(tob.total_qty, 0.0011f);
    BOOST_CHECK(hbook.get_best_px(3, eSide::BID).empty());
}

BOOST_AUTO_TEST_CASE(simple_tob)
{
    using namespace bitfinex;
    using level_based::OrderBookP;
    using level_based::PxQx;

    OrderBookP book;
    PxQx tob;
}

BOOST_AUTO_TEST_SUITE_END()
