#pragma once
#include <cstdint>
#include <cmath>
#include <vector>
#include <string>
#include <boost/version.hpp>
#include <boost/container/flat_map.hpp>

int f1(int x);

// https://docs.bitfinex.com/reference#ws-public-books
// wscat -c wss://api-pub.bitfinex.com/ws/2

// Asking for book checksum:
// { "event": "conf", "flags": "131072" }
// Received Response:
// {"event":"conf","status":"OK"}
// Check checksum will appear periodically:
// [102439,"hb"]

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

namespace bitfenix
{

using channel_tag_t = uint64_t; // Not necessarily parsed, might just the original channelId raw string, e.g. '[100838,'
using qx_t          = float;

enum eSide
{
    BID = 0,
    ASK = 1
};

inline constexpr eSide qx_to_side(qx_t qq) {return qq < 0 ? eSide::ASK : eSide::BID;}

namespace level_based
{
    using px_t = uint32_t;

    struct PxQx
    {
        px_t price_level;
        qx_t total_qty;
    };

    // https://docs.bitfinex.com/reference#ws-public-books
    class OrderBookSideP // Ask or Bid side. Level-based OB
    {
        using LevelMap = boost::container::flat_map<px_t, qx_t>; // ordered
        LevelMap    m_level_map; 
        PxQx        m_last_update {}; // optimization
    public:
        void clear();

        // Overwrites previous quantity at price level
        // new_qty should be a value > 0, regardless of ask/bid
        void assign_level(px_t price_level, qx_t new_qty);
        void erase_level(px_t price_level);

        PxQx get_tob(uint32_t offset);
    };


    class OrderBookP
    {
        OrderBookSideP m_book_sides[2];
    public:
        void clear()
            {
            m_book_sides[eSide::BID].clear();
            m_book_sides[eSide::ASK].clear();
            }
        void assign_level(px_t price_level, qx_t new_qty)
            {
            const auto side = qx_to_side(new_qty);
            m_book_sides[side].assign_level(price_level, std::fabs(new_qty) );
            }
        void erase_level(px_t price_level, eSide side)
            {
            m_book_sides[side].erase_level(price_level);
            }
        PxQx get_tob(eSide side, uint32_t offset)
            {
            m_book_sides[side].get_tob(offset);
            }
    };

} // level_based

// namespace order_based
// {
//     using px_t = float;
//     class OrderBookSideR // Raw order book, i.e. Order-based OB
//     {
//         void clear();
//         void update_(px_t price_level, qx_t new_qty);
//         void erase_level(px_t price_level);
//         void get_tob(uint32_t offset);
//     };
// } // namespace order_based


struct SubscriptionArgs
{
    std::string symbol    {"tBTCUSD"};
    std::string precision {"P0"};  // *P0,...,P4 price aggregation. P0 is 5 significant digits, P4 is just 1 digit
    std::string length    {"250"}; // 1, *25, 100, 250
    std::string freq      {"f0"};  // *f0 (real time), f1 (2 seconds)
    std::string subId     {};

    // { "event": "subscribe", "channel": "book", "symbol": "tBTCUSD", "prec": "P0", "len": "250", "subId":"999" }
    std::string as_json_rpc_request() const;
};

// Intended for use by one WebSocket connection, multiple subscriptions
class MarketDataFeed
{
    // Open-address hash, channel_tag_t -> index
    std::vector<level_based::OrderBookP> m_books;
public:
    void subscribe(const SubscriptionArgs&);
};


} // namespace bitfenix
