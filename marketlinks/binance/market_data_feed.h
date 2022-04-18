#pragma once
#include "marketlinks/common/types.h"
#include "marketlinks/common/buf_desc.h"
#include "marketlinks/common/rest_request.h"

/*
py ->
b1 = cpp.Book(market='BINANCE',  symbol='BTC_USDT' ) -> engine
b2 = cpp.Book(market='BITFINEX', symbol='tBTCUSD' )

class MarketDataFeed : Parser
{
    WebSocket m_ws;
public:
    void rpc_request(RestRequest);

    subscribe(symbol);
}

*/


//class Parser // isolated just for unit test
//{
//public:
//    using Error = TruncatedData;
//    size_t parse_data_and_dispatch_message(char const* data, size_t len);
//private:
//};

namespace binance
{

struct Traits_Example
{
    enum { HANDLE_TICKER = true };
    enum { HANDLE_DEPTH  = false };
    enum { MULTI_BOOK    = true };
};

template<class Traits>
class MarketDataFeed // one web socket, 1..N symbols, 1 book per symbol
{
public:
    void request(RestRequest const&);
private:
    Parser m_parser;
};

class BookMultiplexer
{

};

} // namespace binance
