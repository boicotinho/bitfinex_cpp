#include <boost/test/unit_test.hpp>
#include "core/url.h"

BOOST_AUTO_TEST_SUITE(wss)

BOOST_AUTO_TEST_CASE(url1)
{
    WebSocketURL url("ws://websocket-echo.com");
    BOOST_CHECK_EQUAL(url.host, "websocket-echo.com");
    BOOST_CHECK_EQUAL(url.port, "80");
    BOOST_CHECK_EQUAL(url.hand_shake, "/");
    BOOST_CHECK_EQUAL(url.use_ssl, false);
}

BOOST_AUTO_TEST_CASE(url2)
{
    WebSocketURL url("wss://api-pub.bitfinex.com/ws/2");
    BOOST_CHECK_EQUAL(url.host, "api-pub.bitfinex.com");
    BOOST_CHECK_EQUAL(url.port, "443");
    BOOST_CHECK_EQUAL(url.hand_shake, "/ws/2");
    BOOST_CHECK_EQUAL(url.use_ssl, true);
}

BOOST_AUTO_TEST_CASE(url3)
{
    WebSocketURL url("ws://websocket-echo.com:9999/xx");
    BOOST_CHECK_EQUAL(url.host, "websocket-echo.com");
    BOOST_CHECK_EQUAL(url.port, "9999");
    BOOST_CHECK_EQUAL(url.hand_shake, "/xx");
    BOOST_CHECK_EQUAL(url.use_ssl, false);
}

BOOST_AUTO_TEST_CASE(url4)
{
    WebSocketURL url("ws://www.websocket-echo.com:333/1/2/3/4");
    BOOST_CHECK_EQUAL(url.host, "www.websocket-echo.com");
    BOOST_CHECK_EQUAL(url.port, "333");
    BOOST_CHECK_EQUAL(url.hand_shake, "/1/2/3/4");
    BOOST_CHECK_EQUAL(url.use_ssl, false);
}

BOOST_AUTO_TEST_CASE(url5)
{
    WebSocketURL url("wss://localhost:11/bb");
    BOOST_CHECK_EQUAL(url.host, "localhost");
    BOOST_CHECK_EQUAL(url.port, "11");
    BOOST_CHECK_EQUAL(url.hand_shake, "/bb");
    BOOST_CHECK_EQUAL(url.use_ssl, true);
}

BOOST_AUTO_TEST_CASE(url6)
{
    WebSocketURL url("wss://localhost/bb");
    BOOST_CHECK_EQUAL(url.host, "localhost");
    BOOST_CHECK_EQUAL(url.port, WebSocketURL::DEFAULT_WSS);
    BOOST_CHECK_EQUAL(url.hand_shake, "/bb");
    BOOST_CHECK_EQUAL(url.use_ssl, true);
}

BOOST_AUTO_TEST_CASE(url7)
{
    WebSocketURL url("ws://localhost");
    BOOST_CHECK_EQUAL(url.host, "localhost");
    BOOST_CHECK_EQUAL(url.port, WebSocketURL::DEFAULT_WS);
    BOOST_CHECK_EQUAL(url.hand_shake, "/");
    BOOST_CHECK_EQUAL(url.use_ssl, false);
}

BOOST_AUTO_TEST_CASE(url8)
{
    WebSocketURL url("ws://127.0.0.1:55/cc/aa");
    BOOST_CHECK_EQUAL(url.host, "127.0.0.1");
    BOOST_CHECK_EQUAL(url.port, "55");
    BOOST_CHECK_EQUAL(url.hand_shake, "/cc/aa");
    BOOST_CHECK_EQUAL(url.use_ssl, false);
}

BOOST_AUTO_TEST_SUITE_END()
