#pragma once
#include "bitfinex/order_book_p.h"
#include "bitfinex/order_book_r.h"
#include "bitfinex/types.h"
#include "bitfinex/json_obj.h"
#include "core/gcc_utils.h"
#include "core/fast_parse.h"
#include "core/x_assert.h"
#include <stdexcept>
#include <stdint.h>

namespace bitfinex
{

// Parser for the bitfinex protocol based on fast_parse() functions
// Perf 1,000 x P0 parse message: '[266343,[41698,3,-0.7317539]'
//     0.0 % :          387 cc
//    10.0 % :          399 cc
//    50.0 % :          411 cc
//    75.0 % :          414 cc
//    90.0 % :          417 cc
//    99.0 % :          786 cc
//    99.5 % :        1,068 cc
//    99.8 % :       15,309 cc
//    99.9 % :       24,351 cc
//   100.0 % :       24,351 cc
//   average :          470 cc
class Parser
{
public:
    // Returns 0 if the parser doesn't see a full message.
    // Otherwise return the number of bytes consumed after dispatching just 1 message.
    // The function will not attempt to dispatch more than 1 message.
    // When a message is successfully parsed, the matching OnMessageXXXX() virtual method will be called.
    // Note that when the compiler can see there's only 1 derivation of a class, it should
    // be able to optimize away the virtual calls here and promote them to inline calls.
    // TODO: Maybe this method should include an argument is_raw_book = {true, false, dont_know},
    //       so the method doesn't have to dynamically detect it from the data only.
    size_t parse_data_and_dispatch_message( // may throw(TruncatedData);
                char const* raw_data, // pointer to beginning of received data. don't know if contains partial, full or multiple messages
                char const* end );    // one past end of raw_data

    // Since it will be very rare that a received data will be truncated,
    // throwing exceptions when that happens is OK and simplifies the logic;
    struct TruncatedData : std::runtime_error
        {
        TruncatedData() : std::runtime_error("Parser::TruncatedData") {}
        };

    //======//
protected:  // To be overridden by derived class:
    //======//

    // Events are parsed more slowly because they aren't latency sensitive.
    virtual void on_message_event(const JsonObj&) {}

    // [266343,"hb"]
    virtual void on_message_heartbeat(channel_tag_t) {}

    // Single update message:
    //   [CHANNEL_ID, [PRICE,COUNT,AMOUNT]]
    //   [266343,[41698,3,-0.7317539]]
    // Bulk update message:
    //   [CHANNEL_ID,[ [PRICE,COUNT,AMOUNT], [PRICE,COUNT,AMOUNT], ... ]]
    //   [266343,[[41669,1,0.00057903],[41664,1,0.08608615],[41663,1,0.26620551],[41662,2,0.48411987],[41661,1,0.05],[41660,1,0.25575155],[41658,1,0.01859116],[41657,1,0.30690186],[41655,4,0.52985585],[41654,1,2.11822663],[41653,1,0.42],[41652,2,0.456219],[41651,3,0.21350308],[41648,2,0.606804],[41647,2,1.17590912],[41646,3,0.79012232],[41645,3,0.3149932],[41643,1,0.03],[41641,1,0.01],[41640,2,0.149308],[41639,4,0.77691617],[41638,2,0.06630008],[41637,2,0.2181],[41636,1,0.03],[41635,2,0.24840023],[41670,2,-0.61302395],[41672,1,-0.06002432],[41673,1,-0.060025],[41674,1,-0.120049],[41675,3,-0.38007726],[41676,1,-0.1238],[41677,2,-0.180069],[41679,1,-0.180054],[41681,1,-0.18005075],[41683,1,-1.82767377],[41684,3,-0.34506754],[41685,1,-0.180027],[41686,2,-3.43228823],[41687,2,-0.29007563],[41688,2,-0.750357],[41689,2,-0.35908188],[41691,3,-0.75585496],[41692,4,-0.95047649],[41693,1,-0.754],[41694,2,-0.18603991],[41695,1,-0.1],[41696,1,-0.3869],[41697,2,-0.22604916],[41698,2,-0.628085],[41699,4,-1.01484266]]]
    // https://docs.bitfinex.com/reference#ws-public-books
    virtual void on_message_update_p(
        channel_tag_t        channel,        // channel id integer given by exchange during subscription to the instrument.
        level_based::px_t    price_level,    // Depending on subscr. precision may be: 41785, 41780, 41700, 41000, 40000.
        size_t               num_orders_at_this_price_level,
        qx_side_t            qty_and_side,   // if positive we have a BID order at this qty. If negative, we have an ASK order at fabs(qty).
        bool                 is_bulk_update) // true if this is part of large bulk update message, which gets sent in the beginning of a subscription
        {}

    // Single update message:
    //   [CHANNEL_ID,[ORDER_ID,PRICE,AMOUNT]]
    //   [165356,[86143754888,41646.35995408,0.10224349]]
    // Bulk update message:
    //   165356,[[86143757311,41652,0.24],[..],...]]
    //   [CHANNEL_ID,[[ORDER_ID,PRICE,AMOUNT],[ORDER_ID,PRICE,AMOUNT],...]]
    // https://docs.bitfinex.com/reference#ws-public-raw-books
    virtual void on_message_update_r(
        channel_tag_t        channel,        // channel id integer given by exchange during subscription to the instrument.
        order_based::oid_t   order_id,
        order_based::px_t    order_price,    // Floating point number, exact price for this order, without level-grouping
        qx_side_t            qty_and_side,
        bool                 is_bulk_update) // true if this is part of large bulk update message, which gets sent in the beginning of a subscription
        {}

private:
    static void skip(char const*& pp, char const* end, char expected_char);
    size_t process_single_book_update(channel_tag_t, char const* bgn, char const* end, bool is_bulk);
    size_t process_bulk_book_update(channel_tag_t, char const* bgn, char const* end);
    size_t process_json_event(char const* bgn, char const* end);
};


//=================================================================================================
FORCE_INLINE void Parser::skip(char const*& pp, char const* end, char expected_char)
{
    ASSERT_EQ(*pp, expected_char);
    if(pp >= end)
        throw TruncatedData();
    pp += (pp < end);
}

//=================================================================================================
FORCE_INLINE size_t Parser::parse_data_and_dispatch_message(
        char const* const a_raw_data,
        char const* const a_end )
{
    if(UNLIKELY(a_end == a_raw_data))
        return 0;
    char const* pp = a_raw_data;
    if(LIKELY(*pp == '['))
    {
        // This is the common case, one of:
        //      [266343,[41700,4,-1.48322347]]
        //      [165356,[86143754891,41647.71373492,0.10368902]]
        //      ^
        // Bulk update:
        //      [266343,[[41669,1,0.00057903],[41664,1,0.08608615], ...
        //      [165356,[[86143757311,41652,0.24],[86143768647,41652,0.00006183], ...
        //      ^
        skip(pp, a_end, '[');
        channel_tag_t channel;
        fast_parse(pp, a_end, channel);
        skip(pp, a_end, ',');
        if(UNLIKELY(pp[1] == '[')) // Bulk update. Very rare.
        {
            pp += process_bulk_book_update(channel, pp, a_end);
        }
        else // Single update. This is by far the most common case, 99.99% of the time.
        {
            pp += process_single_book_update(channel, pp, a_end, false);
        }
    }
    else // unlikely event report, starts with '{'
    {
        pp += process_json_event(pp, a_end);
    }
    return pp - a_raw_data;
}

//=================================================================================================
FORCE_INLINE size_t Parser::process_single_book_update(
        channel_tag_t const a_curr_channel,
        char const*   const a_bgn,
        char const*   const a_end,
        bool          const a_is_bulk)
{
    char const* pp = a_bgn;
    // This is the common case, one of:
    //      [266343,[41700,4,-1.48322347]]
    //      [165356,[86143754891,41647.71373492,0.10368902]]
    //              ^
    // Bulk update:
    //      [266343,[[41669,1,0.00057903],[41664,1,0.08608615], ...
    //      [165356,[[86143757311,41652,0.24],[86143768647,41652,0.00006183], ...
    //               ^
    skip(pp, a_end, '[');

    level_based::px_t price_level;

    if(FeedTraits::SUPPORTS_RAW_BOOK) // Compile-time constant, code will be optimized out when we don't need raw books
    {
        // Dynamic detection of raw book feed, when we don't know in advance.
        // It might be advantageous to pass is_raw_book as an argument,
        // as it might be cheaper to determine this from upstream, given the channel id.
        // [165356,[86143754891,41647.71373492,0.10368902]]
        //          ^
        uint64_t maybe_price_or_order_id;
        fast_parse(pp, a_end, maybe_price_or_order_id);
        skip(pp, a_end, ',');
        // It seems that if the first value is 10s of billions large,
        // it cannot be a price, but ought to be the OrderID.
        if(maybe_price_or_order_id >= 10000000000uLL)
        {
            order_based::oid_t order_id = maybe_price_or_order_id;
            // [165356,[86143754891,41647.71373492,0.10368902]]
            //                      ^
            order_based::px_t price;
            fast_parse(pp, a_end, price);
            skip(pp, a_end, ',');
            qx_side_t qs;
            fast_parse(pp, a_end, qs);
            skip(pp, a_end, ']');
            skip(pp, a_end, ']');
            this->on_message_update_r(a_curr_channel, order_id, price, qs, a_is_bulk);
            return pp - a_bgn;
        }
        else
            price_level = maybe_price_or_order_id;
    }
    else // else we only ever parse P0 book updates
    {
        // [266343,[41700,4,-1.48322347]]
        //          ^
        fast_parse(pp, a_end, price_level);
        skip(pp, a_end, ',');
    }

    // P0, level-based book update
    uint32_t order_count;
    fast_parse(pp, a_end, order_count);
    skip(pp, a_end, ',');
    qx_side_t qs;
    fast_parse(pp, a_end, qs);
    skip(pp, a_end, ']');
    if(!a_is_bulk)
        skip(pp, a_end, ']');
    this->on_message_update_p(a_curr_channel, price_level, order_count, qs, a_is_bulk);
    return pp - a_bgn;
}

} // namespace bitfinex
