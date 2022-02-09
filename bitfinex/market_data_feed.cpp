#include "market_data_feed.h"
#include "core/string_utils.h"
#include <iostream>

namespace bitfinex
{

void MarketDataFeed::start_recv_thread(
    std::chrono::nanoseconds const  a_timeout,
    std::string const&              a_url)
{
    if(m_recv_thread.joinable())
        throw std::runtime_error("MarketDataFeed thread already started");

    // Connect
    WebSocketURL parsed_url(a_url);
    m_ws_client = WebSocketClient(parsed_url);

    // Finally start parallel thread to update the book(s)
    m_recv_thread = std::thread(&MarketDataFeed::run_loop_recv_thread, this);
}

void MarketDataFeed::stop_recv_thread() noexcept
{
    if(m_recv_thread.joinable())
    {
        m_ws_client.close();
        m_recv_thread.join();
    }
}

void MarketDataFeed::run_loop_recv_thread()
{
    try
    {
        for(;;)
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
}

Subscription MarketDataFeed::subscribe(SubscriptionConfig const& a_subs_cfg)
{
    PendingSub* psub = nullptr;
    int sub_id = -1;
    {
        std::lock_guard<SleepMutex> lock(m_subscription_mtx);
        sub_id = m_next_sub_guid++;
        psub = &m_pending_subscriptions[sub_id];
    }
    auto const rpc_req = a_subs_cfg.as_json_rpc_request(std::to_string(sub_id));
    m_ws_client.blk_send_str(rpc_req);
    const bool success = psub->ready_sem.WaitForReadySignalTO(std::chrono::seconds(10));

    Subscription new_sub;

    std::lock_guard<SleepMutex> lock(m_subscription_mtx);
    auto it = m_pending_subscriptions.find(sub_id);
    if(it == m_pending_subscriptions.end() || &it->second != psub)
        throw std::logic_error("MarketDataFeed could not find/match pending subscription");
    if(!success)
        return Subscription();
    new_sub = Subscription(std::move(psub->book_p), psub->cfg);
    m_pending_subscriptions.erase(it);
    return new_sub;
}

void MarketDataFeed::unsubscribe(Subscription& a_subs)
{
    auto const& chan_id = a_subs.get_config().channelId;
    std::string const rpc_req = FormatString(R"({"event": "unsubscribe", "chanId":%u})",chan_id);
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
        std::lock_guard<SleepMutex> lock(m_subscription_mtx);
        auto it = m_pending_subscriptions.find(std::atoi(subs_id.c_str()));
        if(it == m_pending_subscriptions.end())
            return;
        PendingSub& sub = it->second;
        sub.cfg.channelId = chan_id;
        sub.book_p = std::make_shared<level_based::OrderBookP>(sub.cfg.channelId);
        m_books_p.insert({chan_id, sub.book_p});
        sub.ready_sem.SignalReady();
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
