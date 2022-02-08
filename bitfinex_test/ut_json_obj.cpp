#include "test_utils.h"
#include "bitfinex/json_obj.h"

BOOST_AUTO_TEST_SUITE(bitfinex)

using bitfenix::JsonObj;

BOOST_AUTO_TEST_CASE(json_obj_parse_conf_status)
{
    const auto jstr = R"({"event":"conf","status":"OK"})";
    JsonObj obj(jstr);
    BOOST_CHECK_EQUAL(obj.get("event"), "conf");
    BOOST_CHECK_EQUAL(obj.get("status"), "OK");
    std::string val;
    BOOST_CHECK_EQUAL(obj.try_get("statu", val), false);
    BOOST_CHECK_EQUAL(obj.try_get("statuss", val), false);
}

BOOST_AUTO_TEST_CASE(json_obj_parse_server_info)
{
    const auto jstr = R"({"event":"info","version":2,"serverId":"ba599674-2ff9-43a0-bb0a-eb3174ab2073","platform":{"status":1}})";
    JsonObj obj(jstr);
    BOOST_CHECK_EQUAL(obj.get("event"), "info");
    BOOST_CHECK_EQUAL(obj.get("version"), "2");
    BOOST_CHECK_EQUAL(obj.get("platform"), R"({"status":1})");
}

BOOST_AUTO_TEST_CASE(json_obj_parse_subscription_info)
{
    const auto jstr = R"({"event":"subscribed","channel":"book","chanId":266343,"symbol":"tBTCUSD","prec":"P0","freq":"F0","len":"25","pair":"BTCUSD"})";
    JsonObj obj(jstr);
    BOOST_CHECK_EQUAL(obj.get("event"), "subscribed");
    BOOST_CHECK_EQUAL(obj.get("channel"), "book");
    BOOST_CHECK_EQUAL(obj.get("chanId"), "266343");
    BOOST_CHECK_EQUAL(obj.get("symbol"), "tBTCUSD");
    BOOST_CHECK_EQUAL(obj.get("prec"), "P0");
    BOOST_CHECK_EQUAL(obj.get("freq"), "F0");
    BOOST_CHECK_EQUAL(obj.get("len"), "25");
    BOOST_CHECK_EQUAL(obj.get("pair"), "BTCUSD");
}

BOOST_AUTO_TEST_CASE(json_obj_copy_ctor)
{
    const auto jstr = R"({"event":"subscribed","channel":"book","chanId":266343,"symbol":"tBTCUSD","prec":"P0","freq":"F0","len":"25","pair":"BTCUSD"})";
    JsonObj obj_src(jstr);
    JsonObj obj(obj_src);
    BOOST_CHECK_EQUAL(obj_src.get("event"), "subscribed");
    BOOST_CHECK_EQUAL(obj.get("event"), "subscribed");
    BOOST_CHECK_EQUAL(obj.get("channel"), "book");
    BOOST_CHECK_EQUAL(obj.get("chanId"), "266343");
    BOOST_CHECK_EQUAL(obj.get("symbol"), "tBTCUSD");
    BOOST_CHECK_EQUAL(obj.get("prec"), "P0");
    BOOST_CHECK_EQUAL(obj.get("freq"), "F0");
    BOOST_CHECK_EQUAL(obj.get("len"), "25");
    BOOST_CHECK_EQUAL(obj.get("pair"), "BTCUSD");
}

BOOST_AUTO_TEST_CASE(json_obj_move_ctor)
{
    const auto jstr = R"({"event":"subscribed","channel":"book","chanId":266343,"symbol":"tBTCUSD","prec":"P0","freq":"F0","len":"25","pair":"BTCUSD"})";
    JsonObj obj_src(jstr);
    JsonObj obj(std::move(obj_src));
    std::string val;
    BOOST_CHECK_EQUAL(obj_src.try_get("event", val), false);
    BOOST_CHECK_EQUAL(obj.get("event"), "subscribed");
    BOOST_CHECK_EQUAL(obj.get("channel"), "book");
    BOOST_CHECK_EQUAL(obj.get("chanId"), "266343");
    BOOST_CHECK_EQUAL(obj.get("symbol"), "tBTCUSD");
    BOOST_CHECK_EQUAL(obj.get("prec"), "P0");
    BOOST_CHECK_EQUAL(obj.get("freq"), "F0");
    BOOST_CHECK_EQUAL(obj.get("len"), "25");
    BOOST_CHECK_EQUAL(obj.get("pair"), "BTCUSD");
}

BOOST_AUTO_TEST_SUITE_END()
