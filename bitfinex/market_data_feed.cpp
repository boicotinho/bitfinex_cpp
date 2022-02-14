#include "market_data_feed.h"
#include "core/string_utils.h"
#include <iostream>

namespace bitfinex
{

void MarketDataFeed::start_recv_thread( std::string const& a_url)
{
    if(++m_started == 1)
    {
        std::cout << "Starting MarketDataFeed...\n";
        // Connect
        WebSocketURL parsed_url(a_url);
        m_ws_client = WebSocketClient(parsed_url);

        // Finally start parallel thread to update the book(s)
        m_recv_thread = std::thread(&MarketDataFeed::run_loop_recv_thread, this);
    }
    else
        std::cout << "Already started MarketDataFeed...\n";
}

void MarketDataFeed::stop_recv_thread() noexcept
{
    if(--m_started == 0)
    {
        std::cout << "Stopping MarketDataFeed...\n";
        m_quit = true;
        m_recv_thread.join();
    }
    else
    {
        std::cout << "MarketDataFeed was already stopped.\n";
    }
}

void MarketDataFeed::run_loop_recv_thread()
{
    std::cerr << "MarketDataFeed recv thread started, build "
              << __DATE__ << " " <<  __TIME__ << " \n";
    try
    {
        while(!m_quit)
        {
            auto const buf = m_ws_client.consume_begin();
            const size_t consumed =
                this->parse_data_and_dispatch_message(buf.data, buf.data + buf.len);
            // Right here these member functions will be called back from parse:
            //      on_message_event()
            //      on_message_update_p()
            m_ws_client.consume_commit(consumed);
        }
    }
    catch(const std::exception& ex)
    {
        std::cerr << "MarketDataFeed recv thread exited: "
                  <<  ex.what() << '\n';
    }
    std::cerr << "MarketDataFeed recv thread exited normally.\n";
}

OrderBookPPtr MarketDataFeed::subscribe(
        SubscriptionConfig const&      a_subs_cfg,
        std::chrono::nanoseconds const a_timeout)
{
    if(!m_started.load())
    {
        std::cerr << "MarketDataFeed::subscribe failed, recv thread ins't started.\n";
        return {};
    }
    int sub_id = -1;
    std::future<OrderBookPPtr> fut;
    {
        std::lock_guard<SleepMutex> lock(m_subscription_mtx);
        sub_id = m_next_sub_guid++;
        fut = m_pending_subscriptions[sub_id].get_future();
    }
    auto const rpc_req = a_subs_cfg.as_json_rpc_request(std::to_string(sub_id));
    m_ws_client.blk_send_str(rpc_req);
    if(std::future_status::ready != fut.wait_for(a_timeout))
        return {};

    return fut.get();
}

void MarketDataFeed::unsubscribe(channel_tag_t a_chan_id)
{
    std::string const rpc_req = FormatString(R"({"event": "unsubscribe", "chanId":%u})",a_chan_id);
    m_ws_client.blk_send_str(rpc_req);
}

// Received subscription response:
void MarketDataFeed::on_message_event(const JsonObj& a_json)
{
    std::string event_type;
    if(!a_json.try_get("event", event_type))
        return;
    if(event_type == "subscribed")
    {
        // {"event":"subscribed","channel":"book","chanId":65219,"symbol":"tBTCUSD","prec":"P0","freq":"F0","len":"25","subId":"0x1122334455667788","pair":"BTCUSD"}
        std::string const&  subs_id = a_json.get("subId");
        channel_tag_t const chan_id = std::atoi(a_json.get("chanId").c_str());
        std::promise<OrderBookPPtr> ob_promise;
        {
            std::lock_guard<SleepMutex> lock(m_subscription_mtx);
            auto it = m_pending_subscriptions.find(std::atoi(subs_id.c_str()));
            if(it == m_pending_subscriptions.end())
                return;
            ob_promise = std::move(it->second);
            m_pending_subscriptions.erase(it);
        }
        OrderBookPPtr new_book(std::make_shared<level_based::OrderBookP>(chan_id));
        if(m_books_p.size() == 0)
            m_single_book = new_book;
        else
            m_single_book.reset();
        m_books_p.insert({chan_id, new_book}); // m_books_p[chan_id] = sub.book_p; //
        ob_promise.set_value(std::move(new_book));
    }
    else if(event_type == "unsubscribed")
    {
        // {"event":"unsubscribed","status":"OK","chanId":173804}
    }
    else if(event_type == "info")
    {
        // {"event":"info","version":2,"serverId":"ba599674-2ff9-43a0-bb0a-eb3174ab2073","platform":{"status":1}}
    }
}

void MarketDataFeed::on_message_update_p(
    channel_tag_t        channel,        // channel id integer given by exchange during subscription to the instrument.
    level_based::px_t    price_level,    // Depending on subscr. precision may be: 41785, 41780, 41700, 41000, 40000.
    size_t               num_orders_at_this_price_level,
    qx_side_t            qty_and_side,   // if positive we have a BID order at this qty. If negative, we have an ASK order at fabs(qty).
    bool                 is_bulk_update) // true if this is part of large bulk update message, which gets sent in the beginning of a subscription
{
    auto* book_ptr = m_single_book.get();
    if(!book_ptr)
    {
        #if (DENSE_MAP_SUPPORTED)
            // TODO: unify so that DenseMap implements STL-like find()
            auto pp = m_books_p.lookup_ptr(channel);
            if(pp)
                book_ptr = pp->get();
        #else
            auto it = m_books_p.find(channel);
            if(it != m_books_p.end())
                book_ptr = it->second.get();
        #endif
    }
    if(UNLIKELY(!book_ptr))
    {
        // Book is not present, yet we get updates for it??
        return;
    }
    if(UNLIKELY(0 == num_orders_at_this_price_level))
        book_ptr->erase_level(price_level, qty_and_side);
    else
        book_ptr->assign_level(price_level, qty_and_side);
}

void MarketDataFeed::on_message_update_r(
    channel_tag_t        channel,        // channel id integer given by exchange during subscription to the instrument.
    order_based::oid_t   order_id,
    order_based::px_t    order_price,    // Floating point number, exact price for this order, without level-grouping
    qx_side_t            qty_and_side,
    bool                 is_bulk_update) // true if this is part of large bulk update message, which gets sent in the beginning of a subscription
{
}

} // namespace bitfinex
