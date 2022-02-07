#pragma once
#include "types.h"

namespace bitfenix
{
// Order-based order book (raw book).
// Incomplete for now.
// Some prototyping indicated there this raw ("R0") order book updates may
// arrive before the level-based book ("P0").
// So we can potentially arbitrate between books of different type for the
// same instrument, chosing the book which has most recent info.
namespace order_based
{
    using px_t = float;

    struct PxQx
    {
        px_t price_level;
        qx_t total_qty;
    };

    class OrderBookSideR // Raw order book, i.e. Order-based OB
    {
        void clear();
        void update_(px_t price_level, qx_t new_qty);
        void erase_level(px_t price_level);
        PxQx get_tob(uint32_t offset, eSide side);
    };

} // namespace order_based

} // namespace bitfenix

