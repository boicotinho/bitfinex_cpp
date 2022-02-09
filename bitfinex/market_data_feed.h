#pragma once
#include "bitfinex/order_book_p.h"
#include "bitfinex/order_book_r.h"
#include "bitfinex/parser.h"
#include "bitfinex/types.h"
#include "bitfinex/subscription.h"
#include "bitfinex/subscription_cfg.h"
#include "web_socket/web_socket_client.h"
#include "core/dense_map.h"
#include "core/hashers.h"
#include "core/gcc_utils.h"
#include <map>
#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <atomic>
#include <thread>

namespace bitfinex
{

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
    struct BookMapTraits
    {
        using Key    = channel_tag_t; // tag
        using Hasher = IdentityHasher<Key>;
        enum { EMPTY_KEY       = 0 };
        enum { MIN_NUM_BUCKETS = 32 };
        using Value  = OrderBookPPtr;
    };
private:
    DenseMap<BookMapTraits>     m_books_p;
    std::thread                 m_recv_thread;
    WebSocketClient             m_ws_client;
    std::atomic_bool            m_quit {};
    SleepMutex                  m_subscription_mtx;
    int                         m_next_sub_guid {1};
    std::map<int, PendingSub>   m_pending_subscriptions;
};

} // namespace bitfinex
