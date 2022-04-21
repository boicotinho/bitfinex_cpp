#pragma once
#include "marketlinks/common/types.h"
#include "marketlinks/common/json_obj.h"
#include "marketlinks/common/rest_request.h"
#include "marketlinks/common/arb_ptr.h"
#include "core/str_view.h"
#include <memory>
#include <stdint.h>
#include <x86intrin.h>

/*
py ->
b1 = cpp.Book(market='BINANCE',  symbol='BTC_USDT' ) -> engine
b2 = cpp.Book(market='BITFINEX', symbol='tBTCUSD' )
*/

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

    struct PreParse
    {
        eRvType type;
        StrView symbol; // ~710 possible
    };

    PreParse determine_type_and_symbol(StrView); // give chance to select a book
    uint_fast32_t parse_and_dispatch_to_derived_class(StrView, PreParse);

    template<class OnTickerFunc> // OnTickerFunc(UpdateId, bid_px,qx, ask_px,qx)
    uint_fast32_t parse_ticker(StrView, OnTickerFunc);

    uint_fast32_t parse_depth(StrView);
    uint_fast32_t parse_trade(StrView);

private:

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

class BookTicker
{
public:
    void update(eSide, Price, Quantity);
private:
    struct Side
    {
        Price    px;
        Quantity qx;
    };
private:
    Side m_sides[2];
};

class MarkerTable
{
public:
    enum { CAPACITY = 256 };
    union CrossCount
    {
        explicit operator bool() const {return !!reg;}
        uint32_t side[2];
        uint64_t reg;
    };
    CrossCount
    get_num_makers_crossed(Price const a_bid_px, Price const a_ask_px) const
    {
        bool const bid_ok = a_bid_px >= m_cached_top_markers[eSide::BID];
        bool const ask_ok = a_ask_px <= m_cached_top_markers[eSide::ASK];
        if(bid_ok & ask_ok)
            return {};
        enum { NUM_SCALARS = sizeof(__m256)/sizeof(Price) };
        CrossCount res {};
        {
            auto const v_px = _mm256_set1_ps(a_bid_px);
            Price const* const p_bgn =
                (Price const*) m_sides[eSide::BID].sorted_trigger_prices.data();
            Price const* const p_end = p_bgn + m_sizes[eSide::BID];
            auto pp = p_bgn;
            do
            {
                auto const byte_mask = _mm256_cmp_ps(*(__m256*)pp, v_px, _CMP_NLT_UQ);
                auto const bit_mask = _mm256_movemask_ps(byte_mask);
                if(bit_mask)
                {
                    auto const first_set_bit = __builtin_ffs(bit_mask) - 1;
                    res.side[eSide::BID] = (pp - p_bgn) + first_set_bit;
                    break;
                }
                pp += NUM_SCALARS;
            } while(pp < p_end);
        }
        {
            auto const v_px = _mm256_set1_ps(a_ask_px);
            Price const* const p_bgn =
                (Price const*) m_sides[eSide::ASK].sorted_trigger_prices.data();
            Price const* const p_end = p_bgn + m_sizes[eSide::ASK];
            auto pp = p_bgn;
            do
            {
                auto const byte_mask = _mm256_cmp_ps(*(__m256*)pp, v_px, _CMP_NLT_UQ);
                auto const bit_mask = _mm256_movemask_ps(byte_mask);
                if(bit_mask)
                {
                    auto const first_set_bit = __builtin_ffs(bit_mask) - 1;
                    res.side[eSide::ASK] = (pp - p_bgn) + first_set_bit;
                    break;
                }
                pp += NUM_SCALARS;
            } while(pp < p_end);
        }
        return res;
    }
    //CrossCount
    //get_num_makers_crossed2(Price const a_bid_px, Price const a_ask_px) const
    //    {
    //    Price undefined;
    //    static constexpr __m128i negate_ask = _mm_set_ps(0, 0x80000000, 0, 0);
    //    auto const v_inp = _mm_set_ps(a_bid_px, a_ask_px, undefined, undefined);
    //    auto const v_neg = _mm_or_ps(v_inp, negate_ask);
    //    auto const v_cmp = _mm_cmpgt_ps(m_cached_top_markers_sse, v_neg);
    //    }
private:
    struct Side
    {
        alignas(sizeof(__m256))
        std::array<Price,     CAPACITY> sorted_trigger_prices;
        std::array<ClientOID, CAPACITY> client_order_ids;
    };
private:
    union{
    __m128   m_cached_top_markers_sse;
    Price    m_cached_top_markers[2];
    };
    uint32_t m_sizes[2] {};
    Side     m_sides[2] {};
};


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
