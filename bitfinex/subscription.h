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
