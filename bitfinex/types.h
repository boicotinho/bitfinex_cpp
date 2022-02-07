#pragma once
#include <cstdint>

namespace bitfenix
{

using channel_tag_t = uint64_t; // Not necessarily parsed, might just the original channelId raw string, e.g. '[100838,'
using qx_t          = float; // Quantity for order books, both level-based and order-based.
// px_t: the price type will depend on wether it's level-based (int), or order-based (float) book.

enum eSide
{
    BID = 0,
    ASK = 1
};

// Determines book side given quantity.
inline constexpr eSide qx_to_side(qx_t qq) {return qq < 0 ? eSide::ASK : eSide::BID;}

} // namespace bitfenix
