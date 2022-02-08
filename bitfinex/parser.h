#pragma once
#include "bitfinex/order_book_p.h"
#include "bitfinex/order_book_r.h"
#include "bitfinex/types.h"

namespace bitfenix
{

// Parser for the bitfinex protocol.
class Parser
{
public:
    // Returns 0 if the parser doesn't see a full message.
    // Otherwise return the number of bytes consumed after dispatching just 1 message.
    // The function will not attempt to dispatch more than 1 message.
    // When a message is successfully parsed, the matching OnMessageXXXX() virtual method will be called.
    // Note that when the compiler can see there's only 1 derivation of a class, it should
    // be able to optimize away the virtual calls here and promote them to inline calls.

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

    size_t parse_data_and_dispatch_message(char const* const a_raw_data, size_t const a_len)
        {
        return 0;
        }
private:
    // Events are parsed more slowly because they aren't latency sensitive.
    virtual void on_message_event()
        {
        }

    // [266343,"hb"]
    virtual void on_message_heartbeat(channel_tag_t) {}

    // TODO: Book checksum message

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
};

// Deals with parsing the bitfinex protocol, in chunks of string data at a time.
// By the nature of TCP, a chunk of data will most often contain just a single, full message.
// However, a chunk may in some cases contain just a fraction of a message,
// and in some other cases a chunk of data may contain multiple messages.
//class ParserSegmented : public Parser
//{
//public:
//    std::pair<char*, size_t> recv_begin();
//    void recv_commit(size_t);
//    // maybe better dealt with by using beast::flat_buffer
//};

} // namespace bitfenix
