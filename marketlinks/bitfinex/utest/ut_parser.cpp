#include "test_utils.h"
#include "marketlinks/bitfinex/parser.h"
#include "core/profile_utils.h"
#include <vector>
#include <string>

using namespace bitfinex;

BOOST_AUTO_TEST_SUITE(bitfinex)

struct TestUpdateP0
{
    channel_tag_t        ch;
    level_based::px_t    px;
    size_t               num_orders;
    qx_side_t            qs;
    bool                 bulk;
};

struct TestUpdateR0
{
    channel_tag_t        ch;
    order_based::oid_t   order_id;
    order_based::px_t    order_price;
    qx_side_t            qs;
    bool                 bulk;
};

class TestParser : public Parser
{
public:
    std::vector<TestUpdateP0>  m_p0_updates_invoked;
    std::vector<TestUpdateR0>  m_r0_updates_invoked;
    std::vector<JsonObj>       m_json_events_invoked;
    std::vector<channel_tag_t> m_heartbeats_invoked;
    size_t parse_string_dispatch_message(std::string const & str)
    {
        const size_t consumed =
            this->parse_data_and_dispatch_message(
                        str.c_str(), str.c_str() + str.size());
        BOOST_CHECK_EQUAL(consumed, str.size());
        return consumed;
    }
private:
    virtual void on_message_event(const JsonObj& jobj) override
    {
        m_json_events_invoked.push_back(jobj);
    }

    virtual void on_message_heartbeat(channel_tag_t ch) override
    {
        m_heartbeats_invoked.push_back(ch);
    }

    virtual void on_message_update_p(
        channel_tag_t        channel,
        level_based::px_t    price_level,
        size_t               num_orders,
        qx_side_t            qty_and_side,
        bool                 is_bulk_update) override
    {
        m_p0_updates_invoked.push_back({
            channel, price_level, num_orders, qty_and_side, is_bulk_update});
    }
    virtual void on_message_update_r(
        channel_tag_t        channel,
        order_based::oid_t   order_id,
        order_based::px_t    order_price,
        qx_side_t            qty_and_side,
        bool                 is_bulk_update) override
    {
        m_r0_updates_invoked.push_back({
            channel, order_id, order_price, qty_and_side, is_bulk_update});
    }
};

BOOST_AUTO_TEST_CASE(parser_p0)
{
    TestParser parser;
    parser.parse_string_dispatch_message("");
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked.size(), 0);

    parser.parse_string_dispatch_message(R"({"event":"conf","status":"OK"})");
    BOOST_REQUIRE_EQUAL(parser.m_json_events_invoked.size(), 1);
    BOOST_CHECK_EQUAL(parser.m_json_events_invoked[0].get("event"),  "conf");
    BOOST_CHECK_EQUAL(parser.m_json_events_invoked[0].get("status"), "OK");

    parser.parse_string_dispatch_message(R"({"event":"info","version":2,"serverId":"ba599674-2ff9-43a0-bb0a-eb3174ab2073","platform":{"status":1}})");
    BOOST_REQUIRE_EQUAL(parser.m_json_events_invoked.size(), 2);
    BOOST_CHECK_EQUAL(parser.m_json_events_invoked[1].get("event"),   "info");
    BOOST_CHECK_EQUAL(parser.m_json_events_invoked[1].get("platform"), R"({"status":1})");

    parser.parse_string_dispatch_message(R"({"event":"subscribed","channel":"book","chanId":266343,"symbol":"tBTCUSD","prec":"P0","freq":"F0","len":"25","pair":"BTCUSD"})");
    BOOST_REQUIRE_EQUAL(parser.m_json_events_invoked.size(), 3);
    BOOST_CHECK_EQUAL(parser.m_json_events_invoked[2].get("event"),  "subscribed");
    BOOST_CHECK_EQUAL(parser.m_json_events_invoked[2].get("chanId"), "266343");
    BOOST_CHECK_EQUAL(parser.m_json_events_invoked[2].get("pair"),   "BTCUSD");

    parser.parse_string_dispatch_message("[266343,[41660,0,1]]");
    BOOST_REQUIRE_EQUAL(parser.m_p0_updates_invoked.size(), 1);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[0].ch,         266343);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[0].px,         41660);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[0].num_orders, 0);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[0].qs,         1);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[0].bulk,       false);

    parser.parse_string_dispatch_message("[266343,[41698,3,-0.7317539]]");
    BOOST_REQUIRE_EQUAL(parser.m_p0_updates_invoked.size(), 2);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[1].ch,         266343);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[1].px,         41698);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[1].num_orders, 3);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[1].qs,         -0.7317539f);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[1].bulk,       false);

    parser.parse_string_dispatch_message("[266343,[[41669,1,0.00057903],[41664,1,0.08608615],[41663,1,0.26620551],[41662,2,0.48411987],[41661,1,0.05],[41660,1,0.25575155],[41658,1,0.01859116],[41657,1,0.30690186],[41655,4,0.52985585],[41654,1,2.11822663],[41653,1,0.42],[41652,2,0.456219],[41651,3,0.21350308],[41648,2,0.606804],[41647,2,1.17590912],[41646,3,0.79012232],[41645,3,0.3149932],[41643,1,0.03],[41641,1,0.01],[41640,2,0.149308],[41639,4,0.77691617],[41638,2,0.06630008],[41637,2,0.2181],[41636,1,0.03],[41635,2,0.24840023],[41670,2,-0.61302395],[41672,1,-0.06002432],[41673,1,-0.060025],[41674,1,-0.120049],[41675,3,-0.38007726],[41676,1,-0.1238],[41677,2,-0.180069],[41679,1,-0.180054],[41681,1,-0.18005075],[41683,1,-1.82767377],[41684,3,-0.34506754],[41685,1,-0.180027],[41686,2,-3.43228823],[41687,2,-0.29007563],[41688,2,-0.750357],[41689,2,-0.35908188],[41691,3,-0.75585496],[41692,4,-0.95047649],[41693,1,-0.754],[41694,2,-0.18603991],[41695,1,-0.1],[41696,1,-0.3869],[41697,2,-0.22604916],[41698,2,-0.628085],[41699,4,-1.01484266]]]");
    BOOST_REQUIRE_GE(parser.m_p0_updates_invoked.size(), 3);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[2].ch,         266343);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[2].px,         41669);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[2].num_orders, 1);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[2].qs,         0.00057903f);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[2].bulk,       true);

    BOOST_REQUIRE_GE(parser.m_p0_updates_invoked.size(), 4);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[3].ch,         266343);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[3].px,         41664);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[3].num_orders, 1);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[3].qs,         0.08608615f);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked[3].bulk,       true);

    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked.back().ch,         266343);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked.back().px,         41699);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked.back().num_orders, 4);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked.back().qs,         -1.01484266f);
    BOOST_CHECK_EQUAL(parser.m_p0_updates_invoked.back().bulk,       true);
    BOOST_REQUIRE_EQUAL(parser.m_p0_updates_invoked.size(), 52);
}

BOOST_AUTO_TEST_CASE(parser_r0)
{
    TestParser parser;
}

class ProfilingParser : public Parser
{
public:
    std::vector<CpuTimeStamp>  m_timings;
    void parse_string_dispatch_message(const char* str)
    {
        auto const bgn = str;
        auto const end = bgn + strlen(str);
        auto const t0 = rdtscp();
        this->parse_data_and_dispatch_message(bgn, end);
        auto const t1 = rdtscp();
        m_timings.emplace_back(t1 - t0);
    }
private:
    virtual void on_message_update_p(
        channel_tag_t        channel,
        level_based::px_t    price_level,
        size_t               num_orders,
        qx_side_t            qty_and_side,
        bool                 is_bulk_update) override
    {
        COMPILER_BARRIER();
        volatile int dummy = 1;
        (void) dummy;
        COMPILER_BARRIER();
    }
    virtual void on_message_update_r(
        channel_tag_t        channel,
        order_based::oid_t   order_id,
        order_based::px_t    order_price,
        qx_side_t            qty_and_side,
        bool                 is_bulk_update) override
    {
        COMPILER_BARRIER();
        volatile int dummy = 1;
        (void) dummy;
        COMPILER_BARRIER();
    }
};

AUTO_TEST_MICROBENCH(profile_parser_p0)
{
    const int NN = 1000;
    ProfilingParser parser;
    parser.m_timings.reserve(NN);
    for(int ii = 0; ii < NN; ++ii)
    {
        parser.parse_string_dispatch_message("[266343,[41698,3,-0.7317539]]");
    }
    std::cout << format_cc_timings_table(parser.m_timings, "P0 parse message: '[266343,[41698,3,-0.7317539]'") << "\n";
}

BOOST_AUTO_TEST_SUITE_END()
