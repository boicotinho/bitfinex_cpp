#pragma once
#include "bitfinex/order_book_p.h"
#include "bitfinex/order_book_r.h"
#include "bitfinex/parser.h"
#include "bitfinex/types.h"
#include "bitfinex/subscription_cfg.h"
#include "core/gcc_utils.h"
// #include "core/dense_map.h" // TODO
#include "web_socket/web_socket_client.h"
#include <cstdint>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <memory>
#include <atomic>
#include <thread>

namespace bitfinex
{

// Stable storage for books. Books aren't allowed to change memory location
// once created, because the python module concurrently acquires locks on books.
using OrderBookPPtr = std::shared_ptr<level_based::OrderBookP>;
class Subscription;

template<class Key, class Data>
using DenseMap = std::unordered_map<Key, Data>; // TODO: Open-address hash,

// Intended for use by one WebSocket connection, possibly multiple subscriptions.
class MarketDataFeed : private Parser
{
public:
    void start_recv_thread(
            std::chrono::nanoseconds timeout = std::chrono::seconds(10),
            std::string const& url = "wss://api-pub.bitfinex.com/ws/2");

    void stop_recv_thread() noexcept;

    // Subscribe/unsubscribe may not be called concurrently with book access methods.
    Subscription subscribe(SubscriptionConfig const&);
    void unsubscribe(Subscription&);

    ~MarketDataFeed() {stop_recv_thread();}
private:
    void run_loop_recv_thread();
    virtual void on_message_event(const JsonObj&) override; // from Parser::
    virtual void on_message_update_p(channel_tag_t, level_based::px_t, size_t, qx_side_t, bool) override;
    virtual void on_message_update_r(channel_tag_t, order_based::oid_t, order_based::px_t, qx_side_t, bool) override;
private:
    struct PendingSub
    {
        Semaphore           ready_sem;
        OrderBookPPtr       book_p;
        SubscriptionConfig  cfg;
    };
private:
    DenseMap<channel_tag_t,
        OrderBookPPtr>          m_books_p;
    std::thread                 m_recv_thread;
    WebSocketClient             m_ws_client;
    std::atomic_bool            m_quit {};
    SleepMutex                  m_subscription_mtx;
    int                         m_next_sub_guid {1};
    std::map<int, PendingSub>   m_pending_subscriptions;
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

    const SubscriptionConfig& get_config() const {return m_cfg;}
};


} // namespace bitfinex
