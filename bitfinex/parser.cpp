#include "parser.h"

namespace bitfinex
{

size_t Parser::process_bulk_book_update(
        channel_tag_t const a_curr_channel,
        char const*   const a_bgn,
        char const*   const a_end)
{
    char const* pp = a_bgn;
    // Need to first check if the whole message is in the buffer, otherwise
    // we would run out of data halfway through processing updates.
    // Our contract is to consume the whole message or nothing.
    // This check is simple but very inneficient, but it's ok because this
    // will only execute very rarely, i.e. right after we subscribe to a new
    // instrument.
    uint32_t close_bracket_count = 0;
    auto bulk_end = pp + 1;
    for(; bulk_end < a_end && 3 != close_bracket_count; ++bulk_end)
    {
        if(*bulk_end == ']')
        {
            ++close_bracket_count;
        }
        else
            close_bracket_count = 0;
    }
    if(close_bracket_count != 3)
        throw TruncatedData(); // refuse to partially consume a bulk update message
    do {
        ++pp; // skip , or initial [
        // [266343,[[41669,1,0.00057903],[41664,1,0.08608615],[...
        //          ^                    ^                    ^
        pp += process_single_book_update(a_curr_channel, pp, a_end, true);
    } while (*pp == ',');
    skip(pp, a_end, ']');
    skip(pp, a_end, ']');
    return pp - a_bgn;
}

size_t Parser::process_json_event(char const* a_bgn, char const* a_end)
{
    auto pp = a_bgn;
    // {"event":"conf","status":"OK"}
    // {"event":"info","version":2,"serverId":"ba599674-2ff9-43a0-bb0a-eb3174ab2073","platform":{"status":1}}
    // {"event":"subscribed","channel":"book","chanId":266343,"symbol":"tBTCUSD","prec":"P0","freq":"F0","len":"25","pair":"BTCUSD"}
    ASSERT_EQ(*pp, '{');
    // Its possible, albeit unlikely, that the data buffer contains more than just
    // the one json object. In that case we don't want to pass garbage to the json parser.
    size_t brackets_depth = 1;
    auto json_end = pp + 1;
    for(; json_end < a_end && brackets_depth; ++json_end)
    {
        if('}' == *json_end)
            --brackets_depth;
        else if('{' == *json_end)
            ++brackets_depth;
    }
    if(brackets_depth)
        throw TruncatedData();
    // Parse just the json
    JsonObj obj(pp, json_end);
    pp = json_end;
    this->on_message_event(obj);
    return pp - a_bgn;
}

} // namespace bitfinex
