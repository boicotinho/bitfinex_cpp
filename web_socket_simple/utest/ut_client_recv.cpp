#include <boost/test/unit_test.hpp>
#include "web_socket_simple/web_socket_client.h"

BOOST_AUTO_TEST_SUITE(wss)

BOOST_AUTO_TEST_CASE(client_recv)
{
    WebSocketClient client(WebSocketURL("wss://api-pub.bitfinex.com/ws/2"));

    // Send subscription JSON rpc
    auto sent = client.blk_send_str(R"({ "event": "subscribe", "channel": "book", "symbol": "tBTCUSD" })");
    BOOST_CHECK_EQUAL(sent, 64);

    // First receive: server info response
    auto recv_buf = client.consume_begin();
    std::string const recvd_str1(recv_buf.data, recv_buf.len);
    BOOST_REQUIRE_GT(recvd_str1.length(), 2);
    client.consume_commit(recv_buf.len);
    // std::cout << recvd_str1 << "\n";
    // {"event":"info","version":2,"serverId":"6d91fa20-316c-42ad-b4a1-0986e9598d3b","platform":{"status":1}}
    std::string const answ_left1  = R"({"event":"info","version":2,"serverId":)";
    std::string const answ_right1 = R"(,"platform":{"status":1}})";
    BOOST_CHECK_EQUAL(recvd_str1.substr(0,answ_left1.length()), answ_left1);
    BOOST_CHECK_EQUAL(recvd_str1.substr(recvd_str1.size() - answ_right1.length()), answ_right1);

    // Second receive: subscription info response
    recv_buf = client.consume_begin();
    std::string const recvd_str2(recv_buf.data, recv_buf.len);
    BOOST_REQUIRE_GT(recvd_str2.length(), 2);
    client.consume_commit(recv_buf.len);
    // std::cout << recvd_str2 << "\n";
    // {"event":"subscribed","channel":"book","chanId":130695,"symbol":"tBTCUSD","prec":"P0","freq":"F0","len":"25","pair":"BTCUSD"}
    std::string const answ_left2  = R"({"event":"subscribed","channel":"book","chanId":)";
    std::string const answ_right2 = R"(,"symbol":"tBTCUSD","prec":"P0","freq":"F0","len":"25","pair":"BTCUSD"})";
    BOOST_CHECK_EQUAL(recvd_str2.substr(0,answ_left2.length()), answ_left2);
    BOOST_CHECK_EQUAL(recvd_str2.substr(recvd_str2.size() - answ_right2.length()), answ_right2);
}

BOOST_AUTO_TEST_SUITE_END()
