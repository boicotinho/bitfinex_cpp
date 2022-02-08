#pragma once
#include "bitfinex/order_book_p.h"
#include "bitfinex/order_book_r.h"
#include "bitfinex/types.h"
#include "core/gcc_utils.h"
#include <cstdint>
#include <string>
#include <vector>

namespace bitfenix
{

struct SubscriptionConfig
{
    std::string symbol    {"tBTCUSD"};
    std::string precision {"P0"};  // *P0,...,P4 price aggregation. P0 is 5 significant digits, P4 is just 1 digit
    std::string length    {"250"}; // 1, *25, 100, 250
    std::string freq      {"f0"};  // *f0 (real time), f1 (2 seconds)
    std::string subId     {};

    // { "event": "subscribe", "channel": "book", "symbol": "tBTCUSD", "prec": "P0", "len": "250", "subId":"999" }
    std::string as_json_rpc_request() const;

    // Supplied once subscribed at the exchange
    std::string channelId;
};


// Intended for use by one WebSocket connection, possibly multiple subscriptions.
class MarketDataFeed
{
    // Open-address hash, channel_tag_t -> index
    std::vector<level_based::OrderBookP> m_books;
public:
    void subscribe(SubscriptionConfig&);
    void unsubscribe(SubscriptionConfig&);
};

} // namespace bitfenix
