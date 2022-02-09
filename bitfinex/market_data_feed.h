#pragma once
#include "bitfinex/order_book_p.h"
#include "bitfinex/order_book_r.h"
#include "bitfinex/parser.h"
#include "bitfinex/types.h"
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
#include <future>

namespace bitfinex
{

// Stable storage for books. Books aren't allowed to change memory location
// once created, because the python module concurrently acquires locks on books.
// This is also used as a subscription
using OrderBookPPtr = std::shared_ptr<level_based::OrderBookP>;


// Intended for use by one WebSocket connection, possibly multiple subscriptions.
class MarketDataFeed : private Parser
{
public:
    void start_recv_thread(std::string const& url = "wss://api-pub.bitfinex.com/ws/2");

    void stop_recv_thread() noexcept;

    // Subscribe/unsubscribe may not be called concurrently with book access methods.
    OrderBookPPtr subscribe( SubscriptionConfig const&,
                            std::chrono::nanoseconds timeout = std::chrono::seconds(10));
    void unsubscribe(channel_tag_t);

    ~MarketDataFeed() {stop_recv_thread();}
private:
    void run_loop_recv_thread();
    virtual void on_message_event(const JsonObj&) override; // from Parser::
    virtual void on_message_update_p(channel_tag_t, level_based::px_t, size_t, qx_side_t, bool) override;
    virtual void on_message_update_r(channel_tag_t, order_based::oid_t, order_based::px_t, qx_side_t, bool) override;
private:
    struct BookMapTraits
    {
        using Key    = channel_tag_t;
        using Value  = OrderBookPPtr;
        using Hasher = IdentityHasher<Key>;
        enum { EMPTY_KEY       = 0 };
        enum { MIN_NUM_BUCKETS = 32 };
    };
private:
    OrderBookPPtr               m_single_book; // optimization for when there's only one book
    DenseMap<BookMapTraits>     m_books_p;
    std::thread                 m_recv_thread;
    WebSocketClient             m_ws_client;
    std::atomic_bool            m_quit {};

    // Subscription synchronization
    using sub_id_t = int;
    std::map< sub_id_t
            , std::promise<
                OrderBookPPtr>> m_pending_subscriptions;
    SleepMutex                  m_subscription_mtx;
    sub_id_t                    m_next_sub_guid {1};
};

} // namespace bitfinex
