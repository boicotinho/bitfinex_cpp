#pragma once
#include "bitfinex/types.h"

namespace bitfenix
{
// Order-based order book (raw book).
// Not implemented for now.
// Some prototyping indicated there this raw ("R0") order book updates may
// arrive before the level-based book ("P0").
// So we can potentially arbitrate between books of different type for the
// same instrument, chosing the book which has most recent info.
namespace order_based
{
    using px_t = float;
    using oid_t = uint32_t;

    struct PxQx
    {
        px_t price_level;
        qx_t total_qty;
    };

    struct TOB
    {
        PxQx side[2];
    };

    class OrderBookSideR // Raw order book, i.e. Order-based OB
    {
        void clear();
        void update_(px_t price_level, qx_t new_qty);
        void erase_level(px_t price_level);
        TOB get_tob(uint32_t offset);
    };

    class OrderBookR
        : public FeedTraits::MaybeMutex // Empty base class optimization when using NullMutex
    {
        OrderBookSideR m_book_sides[2];
    };

} // namespace order_based

} // namespace bitfenix
