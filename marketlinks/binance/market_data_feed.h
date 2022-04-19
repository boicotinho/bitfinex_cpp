#pragma once
#include "marketlinks/common/types.h"
#include "marketlinks/common/json_obk.h"
#include "marketlinks/common/rest_request.h"
#include "core/str_view.h"
#include "core/gcc_utils.h"
#include "core/cmpxchg16b.h"
#include <memory>
#include <stdint.h>

/*
py ->
b1 = cpp.Book(market='BINANCE',  symbol='BTC_USDT' ) -> engine
b2 = cpp.Book(market='BITFINEX', symbol='tBTCUSD' )

class MarketDataFeed : Parser
{
    WebSocket m_ws;
public:
    void rpc_request(RestRequest);

    subscribe(symbol);
}

*/


//class Parser // isolated just for unit test
//{
//public:
//    using Error = TruncatedData;
//    size_t parse_data_and_dispatch_message(char const* data, size_t len);
//private:
//};

namespace binance // spots only, not futures
{

using UpdateId = uint64_t; // u":18,541,854,003. 86.4m milliseconds in 24h

class Parser // Spot only
{
public:
    enum class eRvType {
        UNKNOWN, RESPONSE,
        TICKER,   DEPTH,   TRADE,   // normal data
        TICKER_C, DEPTH_C, TRADE_C, // after SET_PROPERTY "combined"=true
    };

    eRvType determine_rcv_type(StrView);
    StrView determine_symbol(StrView); // ~710 possible

    size_t  parse_ticker(StrView);
    size_t  parse_depth(StrView);
    size_t  parse_trade(StrView);

    // using TruncatedData;
    // using ProtocolError
private:
    virtual void on_response(JsonObj const&);

    virtual void on_ticker( UpdateId    a_update_id // mono incr, with gaps
                          , Price       a_bid_px
                          , Quantity    a_bid_qx
                          , Price       a_ask_px
                          , Quantity    a_ask_qx
                          , StrView     a_symbol
                          ) {}
};

// update-id: 8b + 4b + 4b + 4b + 4b
class BookTicker
{
    struct Side
    {
        Price    px;
        Quantity qx;
    };
    Side     m_sides[2];
};

struct ArbPtr
{
    using PtrT    = BookTicker*;
    using TimeT   = UpdateId;
    using Compare = std::greater<TimeT>;

    bool atomic_update_if_newer(TimeT const a_uid, PtrT const a_ptr)
    {
        static_assert(sizeof(ArbRef) == 16,
            "Please check ArbRef for atomic 128 CAS")
        auto vv = m_ver;
        if(!Compare()(a_uid, vv))
            return false;
        auto pp = m_ptr;
        return cmpxchg16b(this, vv, pp, a_uid, a_ptr);
    }
private:
    TimeT volatile m_ver {}; // 18,541,854,003 : 35 bits, max 34.3B
    PtrT           m_ptr {};
} PACK128();



// One web socket (will only ever be serviced by 1 service -> 1 thread with 1 epoll or spin)
// 1..N symbols,
// 1 book per symbol,
// 1..3 streams per symbol (ticker, depth, trades)
class MarketDataFeed
    : private Parser
{
public:
    void subscribe();

public:
    void send_request(RestRequest const&);

private:
    virtual void on_ticker( UpdateId    a_update_id // mono incr, with gaps
                          , Price       a_bid_px
                          , Quantity    a_bid_qx
                          , Price       a_ask_px
                          , Quantity    a_ask_qx
                          , StrView     a_symbol
                          ) override;
private:
    struct Cold
    {

    };
private:
    std::unique_ptr<Cold> m_cold;
};

class BookMultiplexer
{

};

} // namespace binance
