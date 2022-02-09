#include "market_data_feed.h"

// Asking for book checksum:
// { "event": "conf", "flags": "131072" }
// Received Response:
// {"event":"conf","status":"OK"}
// Check checksum will appear periodically:
// [266343,"hb"]

// Sending a subscription request:
// { "event": "subscribe", "channel": "book", "symbol": "tBTCUSD" }

// Received request response:
// {"event":"info","version":2,"serverId":"ba599674-2ff9-43a0-bb0a-eb3174ab2073","platform":{"status":1}}

// Received subscription response:
// {"event":"subscribed","channel":"book","chanId":266343,"symbol":"tBTCUSD","prec":"P0","freq":"F0","len":"25","pair":"BTCUSD"}

// Initial Bulk update:
// [CHANNEL_ID,[ [PRICE,COUNT,AMOUNT], [PRICE,COUNT,AMOUNT], ... ]]
// [266343,[[41669,1,0.00057903],[41664,1,0.08608615],[41663,1,0.26620551],[41662,2,0.48411987],[41661,1,0.05],[41660,1,0.25575155],[41658,1,0.01859116],[41657,1,0.30690186],[41655,4,0.52985585],[41654,1,2.11822663],[41653,1,0.42],[41652,2,0.456219],[41651,3,0.21350308],[41648,2,0.606804],[41647,2,1.17590912],[41646,3,0.79012232],[41645,3,0.3149932],[41643,1,0.03],[41641,1,0.01],[41640,2,0.149308],[41639,4,0.77691617],[41638,2,0.06630008],[41637,2,0.2181],[41636,1,0.03],[41635,2,0.24840023],[41670,2,-0.61302395],[41672,1,-0.06002432],[41673,1,-0.060025],[41674,1,-0.120049],[41675,3,-0.38007726],[41676,1,-0.1238],[41677,2,-0.180069],[41679,1,-0.180054],[41681,1,-0.18005075],[41683,1,-1.82767377],[41684,3,-0.34506754],[41685,1,-0.180027],[41686,2,-3.43228823],[41687,2,-0.29007563],[41688,2,-0.750357],[41689,2,-0.35908188],[41691,3,-0.75585496],[41692,4,-0.95047649],[41693,1,-0.754],[41694,2,-0.18603991],[41695,1,-0.1],[41696,1,-0.3869],[41697,2,-0.22604916],[41698,2,-0.628085],[41699,4,-1.01484266]]]

// Delta updates throughout the session:
// [CHANNEL_ID, [PRICE,COUNT,AMOUNT]]
// [266343,[41698,3,-0.7317539]]
// [266343,[41669,0,1]]
// [266343,[41677,0,-1]]

namespace bitfinex
{

void MarketDataFeed::stop_network_thread() noexcept
{

}

void MarketDataFeed::start_network_thread(
        std::chrono::nanoseconds const  a_timeout,
        std::string const&              a_url)
{

}

} // namespace bitfinex
