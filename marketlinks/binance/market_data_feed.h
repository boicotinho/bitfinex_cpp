#pragma once

/*
py ->
b1 = cpp.Book(market='BINANCE',  symbol='BTC_USDT' ) -> engine
b2 = cpp.Book(market='BITFINEX', symbol='tBTCUSD' )

struct BufDesc { void* data; size_t len; }; // alias stringview

class RestRequest
{
    std::string m_msg;
public:
    enum class eKeyword {Null, True, False};
    RestRequest& set(string key, string); // will add "quotes"
    RestRequest& set(string key, vector<string>); // will add ["", ""]
    RestRequest& set(string key, int64_t);
    RestRequest& set(string key, double);
    RestRequest& set(string key, eKeyword);

    BufDesc get_message() const;

    //using response_token = uint32_t;
};

class MarketDataFeed : Parser
{
    WebSocket m_ws;
public:
    void rpc_request(RestRequest);

    subscribe(symbol);
}

*/
namespace binance
{

} // namespace binance
