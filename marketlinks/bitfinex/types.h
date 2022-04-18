#pragma once
#include "core/thread_utils.h"
#include "marketlinks/common/types.h"
#include <cstdint>
#include <mutex>

namespace bitfinex
{

using channel_tag_t = uint32_t; // Not necessarily parsed, might just the original channelId raw string, e.g. '[100838,'
using qx_t          = float; // Quantity for order books, both level-based and order-based. Always positive.
using qx_side_t     = float; // Quantity with side information overloaded: if negative: ASK, if positive: BID
// px_t: the price type will depend on wether it's level-based (int), or order-based (float) book.


// Determines book side given quantity.
inline constexpr eSide qx_to_side(qx_side_t qq) {return qq < 0 ? eSide::ASK : eSide::BID;}

// TODO:
// class QxSide {
//  constexpr QxSide(qx_side_t);
//  constexpr eSide side() const {return qq < 0 ? eSide::ASK : eSide::BID;}
//  constexpr operator qx_t() const {return ::fabs()}
// };


// This is a compile-time only place-holder for traits.
// For now, we only need 1 set of traits for the whole market data class chain.
// Once we need variants such as c++ only market data feeds, raw books,
// support for funding currencies etc, then every class in the market data feed
// chain (order book, parsers etc) should be promoted to template classes which
// follow the well-known traits design patter seen in e.g. STL. This allow us to
// follow the C++ motto and only pay for what you need.
struct FeedTraits
{
    // Because our python module does polling calls into our order books from
    // an external thread, we need a spinning (or sleeping-) mutex.
    // Later on however, it would be advantageous if our thread calls into the
    // trading strategy when there's a relevant market data event, greatly
    // improving performance.
    // Replacing MaybeMutex with NullMutex will completely remove the code
    // and data that would otherwise be needed for thread safety.
    using MaybeMutex     = SpinMutex;
    using MaybeLockGuard = std::lock_guard<SpinMutex>;


    // Raw book not needed for now.
    // But once needed, SUPPORTS_RAW_BOOK should be promoted to a template argument.
    // The intention is to keep this a compile-time argument, allowing compiler
    // optimizations to eliminate code.
    enum { SUPPORTS_RAW_BOOK = false };

    // The feed received from the exchange is assumed to be correct,
    // so time is not spent doing error checks on the protocol.
    enum { ERROR_CHECK_FEED = false };
};

} // namespace bitfinex
