#include <boost/test/unit_test.hpp>
#include "web_socket/web_socket_client.h"

BOOST_AUTO_TEST_SUITE(wss)

BOOST_AUTO_TEST_CASE(client_recv)
{
    WebSocketClient client(WebSocketURL("wss://api-pub.bitfinex.com/ws/2"));
    auto sent = client.blk_send_str(R"({ "event": "subscribe", "channel": "book", "symbol": "tBTCUSD" })");
    BOOST_CHECK_EQUAL(sent, 64);
    auto recv_buf = client.consume_begin();
    std::string const recvd_str(recv_buf.data, recv_buf.len);
    // std::cout << recvd_str << "\n";
    // {"event":"info","version":2,"serverId":"6d91fa20-316c-42ad-b4a1-0986e9598d3b","platform":{"status":1}}
    std::string const answ_left  = R"({"event":"info","version":2,"serverId":)";
    std::string const answ_right = R"(,"platform":{"status":1}})";
    BOOST_CHECK_EQUAL(recvd_str.substr(0,answ_left.length()), answ_left);
    BOOST_CHECK_EQUAL(recvd_str.substr(recvd_str.size() - answ_right.length()), answ_right);
}

BOOST_AUTO_TEST_SUITE_END()
