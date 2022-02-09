#pragma once
#include "bitfinex/types.h"
#include "bitfinex/order_book_p.h"
#include "bitfinex/order_book_r.h"
#include "bitfinex/subscription_cfg.h"

namespace bitfinex
{

// Stable storage for books. Books aren't allowed to change memory location
// once created, because the python module concurrently acquires locks on books.
using OrderBookPPtr = std::shared_ptr<level_based::OrderBookP>;

// API meant for python, who currently queries the book from another thread (slow)
// instead of accessing it from a callback off the network thread (fast).
class Subscription
{
    OrderBookPPtr m_book_p;
public:
    Subscription() = default;
    Subscription(OrderBookPPtr const& book) : m_book_p(book) {}

    channel_tag_t channel_id() const {return m_book_p ? m_book_p->channel_id() : 0;}

    level_based::TOB get_tob(uint32_t depth = 0) const
        {
        if(UNLIKELY(!m_book_p))
            return level_based::TOB::Zero();
        return m_book_p->get_tob(depth);
        }
};

} // namespace bitfinex
