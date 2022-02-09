#include "test_utils.h"
#include "bitfinex/market_data_feed.h"
#include "core/profile_utils.h"

using namespace bitfinex;

BOOST_AUTO_TEST_SUITE(bitfinex)

BOOST_AUTO_TEST_CASE(market_data_feed)
{
    MarketDataFeed md_feed;
    md_feed.start_recv_thread();

    SubscriptionConfig cfg;
    cfg.symbol = "tBTCUSD";
    auto ob = md_feed.subscribe(cfg);
    for(int ii = 0; ii < 20; ++ii)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto tob = ob->get_tob();
        std::cerr << cfg.symbol << " " << tob.to_string() << "\n";
    }
    auto tob = ob->get_tob();
    BOOST_CHECK(tob.has_both());
    BOOST_CHECK_GT(tob.bid().price_level, 30000);
    BOOST_CHECK_LT(tob.ask().price_level, 80000);
    BOOST_CHECK_LT(tob.spread(), 100);
}

BOOST_AUTO_TEST_SUITE_END()
