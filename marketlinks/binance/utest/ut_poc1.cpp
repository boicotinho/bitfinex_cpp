#include <boost/test/unit_test.hpp>
#include "web_socket/web_socket_client.h"
#include "core/profile_utils.h"
#include "core/string_utils.h"

// self.binance_futures_ws_address = "wss://fstream.binance.com"
// self.binance_spot_ws_address = "wss://stream.binance.com:9443"

BOOST_AUTO_TEST_SUITE(binance)

// wss://stream.binance.com:9443/ws/bnbbtc@depth
BOOST_AUTO_TEST_CASE(simple_print)
{
    WebSocketURL parsed_url("ws://stream.binance.com:9443/ws");
    WebSocketClient ws_client = WebSocketClient(parsed_url);
    try
    {
        // Individual Symbol Book Ticker Streams : <symbol>@bookTicker
        // All Book Tickers Stream               : !bookTicker
        // Aggregate Trade Streams               : <symbol>@aggTrade
        // Trade Streams                         : <symbol>@trade
        // Partial Book Depth Streams            : <symbol>@depth<levels> OR <symbol>@depth<levels>@100ms
        // Diff. Depth Stream                    : <symbol>@depth         OR <symbol>@depth@100ms
        auto const rpc_req = FormatString(
            R"({"method": "SUBSCRIBE", "params": ["btcusdt@trade","btcusdt@bookTicker"], "id": 1})");

        ws_client.blk_send_str(rpc_req);

        for(;;)
        {
            auto const buf = ws_client.consume_begin();
            std::string data(buf.data, buf.len);
            std::cout << "\n" << data << "\n";
            const size_t consumed = buf.len;
            ws_client.consume_commit(consumed);
        }
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Exception: " <<  ex.what() << '\n';
    }
}

BOOST_AUTO_TEST_SUITE_END()
