#include "market_data_feed.h"

namespace binance
{

void MarketDataFeed::on_ticker( UpdateId    a_update_id // mono incr, with gaps
                              , Price       a_bid_px
                              , Quantity    a_bid_qx
                              , Price       a_ask_px
                              , Quantity    a_ask_qx
                              , StrView     a_symbol
                              )
{

}

size_t MarketDataFeed::on_recv(StrView)
{
    return 0;
}

} // namespace binance
