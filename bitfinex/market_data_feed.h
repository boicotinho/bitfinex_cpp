#pragma once
#include "bitfinex/order_book_p.h"
#include "bitfinex/order_book_r.h"
#include "bitfinex/types.h"
#include "bitfinex/subscription_cfg.h"
#include "core/gcc_utils.h"
#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <thread>

namespace bitfenix
{

// Stable storage for books. Books aren't allowed to change memory location
// once created, because the python module concurrently acquires locks on books.
using OrderBookPPtr = std::shared_ptr<level_based::OrderBookP>;
class Subscription;

// Intended for use by one WebSocket connection, possibly multiple subscriptions.
class MarketDataFeed
{
    // TODO: Open-address hash, channel_tag_t -> index
    std::vector<OrderBookPPtr>  m_books_p;
    std::thread                 m_network_thread;
    // TODO: use beast::flat_buffer
public:
    void start_network_thread(
            std::chrono::nanoseconds timeout = std::chrono::seconds(10),
            std::string const& url = "wss://api-pub.bitfinex.com/ws/2");

    void stop_network_thread() noexcept;

    // Subscribe/unsubscribe may not be called concurrently with book access methods.
    Subscription subscribe(SubscriptionConfig&);
    void unsubscribe(Subscription&);

    ~MarketDataFeed() {stop_network_thread();}
};

// API meant for python, who currently queries the book from another thread (slow)
// instead of accessing it from a callback off the network thread (fast).
class Subscription
{
    OrderBookPPtr       m_book_p;
    SubscriptionConfig  m_cfg;
public:
    Subscription() = default;
    Subscription(OrderBookPPtr const& book, SubscriptionConfig const& cfg)
        : m_book_p(book), m_cfg(cfg) {}

    level_based::TOB get_tob(uint32_t depth = 0) const
        {
        if(UNLIKELY(!m_book_p))
            return level_based::TOB::Zero();
        return m_book_p->get_tob(depth);
        }
};


} // namespace bitfenix
