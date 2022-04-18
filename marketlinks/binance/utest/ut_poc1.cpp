#include <boost/test/unit_test.hpp>
#include "web_socket/web_socket_client.h"
#include "core/profile_utils.h"
#include "core/string_utils.h"
#include <libwebsockets.h>
//#include <wolfssl/ssl.h>

// self.binance_futures_ws_address = "wss://fstream.binance.com"
// self.binance_spot_ws_address = "wss://stream.binance.com:9443"
// wss://fstream.binance.com/stream?streams=btcusdt@depth@0ms/btcusdt@bookTicker/btcusdt@aggTrade
//      btcusdt@depth@0ms
//      btcusdt@bookTicker
//      btcusdt@aggTrade

BOOST_AUTO_TEST_SUITE(binance)

// Lws must have been built with
//  LWS_ROLE_WS=1
//  LWS_WITH_SECURE_STREAMS=1
//  LWS_WITHOUT_EXTENSIONS=0
// If you want to mimic OpenSSL behavior of having `SSL_connect` succeed even if
// verifying the server fails and reducing security you can do this by calling:
//
// wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_NONE, NULL);
//
// before calling `wolfSSL_new();`. Though it's not recommended.
// https://libwebsockets.org/git/libwebsockets/tree/minimal-examples/client/binance

BOOST_AUTO_TEST_CASE(with_websockets_wolfssl)
{
    //wolfSSL_CTX_SetIORecv(0,0);
    //auto recv_client = [](struct WOLFSSL* ssl, char* buff, int sz, void* ctx)
    //wolfSSL_SetIORecv();

    // this is a thread contex, processing one event loop
    lws_context_creation_info ws_info {};

    // request for custom epoll loop
    struct EpollSet
    {
        int m_epoll_fd {};
        //lws_pollfd	pollfds[64];
        //int			count_pollfds;
    };
    static EpollSet g_epoll_set {};
    void* foreign_loops[1] = {&g_epoll_set};
    ws_info.foreign_loops = foreign_loops;

    // define our connection info
    struct ConnectionInfo
    {
        // more or less required by libwebsockets:
        lws_sorted_usec_list_t	sul {}; // schedule connection retry
        lws*                    wsi {};
        uint16_t		        retry_count {};
    };

    auto on_my_recv = [] ( struct lws*                 wsi
                         , enum lws_callback_reasons   reason
                         , void*                       a_conn_info
                         , void*                       recv_data
                         , size_t                      recv_len
                         ) -> int
    {
        auto const conn_info = (ConnectionInfo*) a_conn_info;
        switch (reason)
        {
            case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
                BOOST_FAIL("CLIENT_CONNECTION_ERROR: "
                     << (const char*)recv_data );
                goto do_retry;
                break;

            case LWS_CALLBACK_CLIENT_RECEIVE:
                std::cerr << std::string((char*)recv_data, recv_len) << "\n";
                break;

            case LWS_CALLBACK_CLIENT_ESTABLISHED:
                std::cerr << "established\n";
                //lws_sul_schedule(lws_get_context(wsi), 0, &conn_info->sul_hz,sul_hz_cb, LWS_US_PER_SEC);
                conn_info->wsi = wsi;
                break;

            case LWS_CALLBACK_CLIENT_CLOSED:
                //lws_sul_cancel(&conn_info->sul_hz);
                goto do_retry;

            default:
                break;
        }
        return lws_callback_http_dummy(wsi, reason, a_conn_info, recv_data, recv_len);
    do_retry:
        BOOST_FAIL("on_my_recv failed, will not try reconnect");
        //if (lws_retry_sul_schedule_retry_wsi(
        //        wsi,
        //        &conn_info->sul,
        //        connect_client,
        //        &conn_info->retry_count))
        //{
        //    BOOST_FAIL("connection attempts exhausted");
        //    //interrupted = 1;
        //}
        return 0;
    };

    // give libwebsockets our callback for processing inbound data.
    // a callback is called a "protocol"
    const lws_protocols protocols[] = {
        { "lws-minimal-client", on_my_recv, 0, 0, 0, NULL, 0 },
        LWS_PROTOCOL_LIST_TERM
    };
    ws_info.protocols = protocols;


    // extensions
    static const lws_extension extensions[] =
    {
        {
            "permessage-deflate",
            lws_extension_callback_pm_deflate, // does malloc (!)
            "permessage-deflate"
            "; client_no_context_takeover"
            "; client_max_window_bits"
        },
        { NULL, NULL, NULL}
    };
    ws_info.extensions = extensions;


    // rest of client init
    ws_info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    ws_info.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
    ws_info.fd_limit_per_thread = 1 + 1 + 1;


    // Wolfssl specific, explicit root CA trust
    static const char * const ca_pem_digicert_global_root =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n"
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n"
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n"
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n"
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n"
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n"
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n"
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n"
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n"
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n"
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n"
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n"
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n"
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n"
    "-----END CERTIFICATE-----\n";
    ws_info.client_ssl_ca_mem = ca_pem_digicert_global_root;
    ws_info.client_ssl_ca_mem_len = (unsigned int)strlen(ca_pem_digicert_global_root);

    // connect
    static lws_context* context = lws_create_context(&ws_info);
    BOOST_REQUIRE_MESSAGE(context, "lws_create_context failed");

    auto on_connect_client = [](lws_sorted_usec_list_t* sul)
    {
        ConnectionInfo* mco = lws_container_of(sul, ConnectionInfo, sul);
        lws_client_connect_info i {};

        i.context = context;
        i.port = 443;
        i.address = "fstream.binance.com";
        i.path = "/stream?"
            "streams="
            //"btcusdt@depth@0ms/"
            "btcusdt@bookTicker/"
            //"btcusdt@aggTrade"
            ;
        i.host = i.address;
        i.origin = i.address;
        i.ssl_connection = LCCSCF_USE_SSL | LCCSCF_PRIORITIZE_READS;
        i.protocol = NULL;
        i.local_protocol_name = "lws-minimal-client";
        i.pwsi = &mco->wsi;

        static const uint32_t backoff_ms[] = { 1000, 2000, 3000, 4000, 5000 };
        static const lws_retry_bo_t retry = {
            /*.retry_ms_table			= */ backoff_ms,
            /*.retry_ms_table_count	    = */ LWS_ARRAY_SIZE(backoff_ms),
            /*.conceal_count			= */ LWS_ARRAY_SIZE(backoff_ms),
            /*.secs_since_valid_ping	= */ 400, /* force PINGs after secs idle */
            /*.secs_since_valid_hangup  = */ 400, /* hangup after secs idle */
            /*.jitter_percent			= */ 0,
        };
        i.retry_and_idle_policy = &retry;
        i.userdata = mco;

        if (!lws_client_connect_via_info(&i))
        {
            BOOST_FAIL("Connection failed (no retry)");
            /*
            * Failed... schedule a retry... we can't use the _retry_wsi()
            * convenience wrapper api here because no valid wsi at this
            * point.
            */
            //if (lws_retry_sul_schedule(context, 0, sul, &retry,
            //            on_connect_client, &mco->retry_count))
            //{
            //    lwsl_err("%s: connection attempts exhausted\n", __func__);
            //    interrupted = 1;
            //}
        }
    };

    ConnectionInfo g_conn_info;

    // schedule the first client connection attempt to happen immediately
    lws_sul_schedule(context, 0, &g_conn_info.sul, on_connect_client, 1);

    int nn = 0;
    while (nn >= 0) // && !interrupted)
        nn = lws_service(context, 0);

    lws_context_destroy(context);
}

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
        // very frequent, real time?             : <symbol>@depth@0ms

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
