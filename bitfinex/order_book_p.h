#pragma once
#include "bitfinex/types.h"
#include "core/gcc_utils.h"
#include <cstdint>
#include <string>
#include <cmath>
#include <boost/container/flat_map.hpp>

namespace bitfenix
{

// Level-based book definitions (non-raw book).
namespace level_based
{
    using px_t = uint32_t; // Price type for the level-based order book.

    struct PxQx
    {
        px_t price_level;
        qx_t total_qty;

        constexpr bool empty() const {return price_level == 0;}
        explicit constexpr operator bool() const {return empty();}
    };

    // Since order deletion does not provide a market opportunity
    // which wasn't there, no strategy should be generating
    // trades on order deletion, only on order insertion, and only
    // those order insertions that improve the top of the book.
    // Looking at just the insertion performance:
    //  Perf 1,000,000 x flat_map insert random
    //      0.0 % :           52 cc
    //     10.0 % :          128 cc
    //     50.0 % :          136 cc
    //     75.0 % :          136 cc
    //     90.0 % :          144 cc
    //     99.0 % :          200 cc
    //     99.5 % :          328 cc
    //     99.8 % :          396 cc
    //     99.9 % :          828 cc
    //    100.0 % :      923,284 cc
    //    average :          158 cc
    // Possible optimization 1:
    //      When the market data tick arrives, don't save it in the flat_map yet.
    //      Instead, save the new update at: PxQx m_last_update;
    //      Process any callback into the trading engine;
    //      If/when the callback asks for the top of book, index ii,
    //      jump to PxQx at index [ii] into the vector, compare that price
    //      with m_last_update; If better return PxQx;
    //      else decrement iterator and compare again.
    // Possible optomization 2:
    //      If user can supply the maximum depth D of book he's interested in,
    //      we can have a vector with length D besides the flat_map.
    //      During the market data tick, we can update just the small vector.
    //      When user asks for top of book, we can just give a pointer to the
    //      whole small vector.
    //      Before calling the new socket recv(), we can update the flat_map.
    // https://docs.bitfinex.com/reference#ws-public-books
    //      wscat -c wss://api-pub.bitfinex.com/ws/2

    // One Side (bid/ask) of an order book for a single instrument
    class OrderBookSideP
    {
        using LevelMap = boost::container::flat_map<px_t, qx_t>; // ordered
        LevelMap m_level_map;
    public:
        void clear() {m_level_map.clear();}

        // Overwrites previous quantity at price level
        // new_qty should be a value > 0, regardless of ask/bid
        void assign_level(px_t pl, qx_t new_qty) {m_level_map.emplace(pl, new_qty);}
        void erase_level(px_t pl) {m_level_map.erase(pl);}

        // In principle, having generated code here that works for both buy and sell is
        // better than templating LevelMap to use inverted comparator (std::greater<>)
        // because the latter will generate twice the ammount of code and hence more
        // L1i cache pressure, more branch mispredictions, more cache miss penalties.
        PxQx get_tob(uint32_t offset, eSide side)
            {
            size_t const sz = m_level_map.size();
            if(UNLIKELY(offset >= sz))
                return {0,0};
            auto it = m_level_map.begin();
            std::advance(it, eSide::ASK == side ? offset : (sz-1-offset));
            return {it->first, it->second};
            }
    };

    // Order book (both sides) for a single instrument,
    class OrderBookP
    {
        OrderBookSideP m_book_sides[2];
    public:
        void clear()
            {
            m_book_sides[eSide::BID].clear();
            m_book_sides[eSide::ASK].clear();
            }
        void assign_level(px_t price_level, qx_t new_qty)
            {
            const auto side = qx_to_side(new_qty);
            m_book_sides[side].assign_level(price_level, std::fabs(new_qty) );
            }
        void erase_level(px_t price_level, eSide side)
            {
            m_book_sides[side].erase_level(price_level);
            }
        PxQx get_tob(eSide side, uint32_t offset)
            {
            m_book_sides[side].get_tob(offset, side);
            }
    };

} // namespace level_based
} // namespace bitfenix
