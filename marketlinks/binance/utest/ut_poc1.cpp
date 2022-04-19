#include <boost/test/unit_test.hpp>
#include "web_socket/web_socket_client.h"
#include "core/profile_utils.h"
#include "core/string_utils.h"
#include "core/profile_utils.h"
#include <chrono>
#include <libwebsockets.h>
//#include <wolfssl/ssl.h>
#include <sys/epoll.h>
#include <signal.h>
#include <boost/utility/string_view.hpp>

// self.binance_futures_ws_address = "wss://fstream.binance.com"
// self.binance_spot_ws_address = "wss://stream.binance.com:9443"
// wss://fstream.binance.com/stream?streams=btcusdt@depth@0ms/btcusdt@bookTicker/btcusdt@aggTrade
//      btcusdt@depth@0ms
//      btcusdt@bookTicker
//      btcusdt@aggTrade

// extern/libwebsockets/READMEs/README.event-loops-intro.md
// extern/libwebsockets/minimal-examples-lowlevel/http-server/minimal-http-server-eventlib-custom/minimal-http-server.c

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

enum class eMode
{
    SPIN,
    POLL,
    EPOLL,
};

bool        g_print_md      = false;
size_t      g_num_conns     = 10;
int         g_epoll_wait_ms = 0;
size_t      g_test_time_s   = 15;
eMode const g_ep_mode       = eMode::EPOLL;

size_t      g_msg_recv      = 0;
bool        g_quit          = false;
void sigint_handler(int sig)
{
	g_quit = true;
}

struct EpollSet
{
    int m_epoll_fd {-1};
    epoll_event ep_events[64];
    // poll impl, rather than epoll, just for test
    lws_pollfd pollfds[64];
    int count_pollfds;
};
struct pt_eventlibs_custom {
	EpollSet *io_loop;
};

lws_pollfd* custom_poll_find_fd(EpollSet *cpcx, lws_sockfd_type fd)
{
	int n;

	for (n = 0; n < cpcx->count_pollfds; n++)
		if (cpcx->pollfds[n].fd == fd)
			return &cpcx->pollfds[n];

	return NULL;
}

int custom_poll_add_fd(EpollSet *cpcx, lws_sockfd_type fd, int events)
{
	lwsl_info("%s: ADD fd %d, ev 0x%x\n", __func__, fd, events);

    //if (g_ep_mode == eMode::POLL)
    {
    	lws_pollfd *pfd = custom_poll_find_fd(cpcx, fd);
        if (pfd) {
            lwsl_info("%s: ADD fd %d already in ext table\n", __func__, fd);
            return 1;
        }

        if (cpcx->count_pollfds == LWS_ARRAY_SIZE(cpcx->pollfds)) {
            lwsl_info("%s: no room left\n", __func__);
            return 1;
        }

        pfd = &cpcx->pollfds[cpcx->count_pollfds++];
        pfd->fd = fd;
        pfd->events = (short)events;
        pfd->revents = 0;
    }
    if (g_ep_mode == eMode::EPOLL)
    //else
    {
        epoll_event evt {};
        evt.data.fd = fd;
        evt.events = events | EPOLLET; // events = POLLIN = 1, EPOLLIN also = 1
        int res = epoll_ctl(cpcx->m_epoll_fd, EPOLL_CTL_ADD, fd, &evt);
        int uu = 32;
    }

	return 0;
}

int
custom_poll_del_fd(EpollSet *cpcx, lws_sockfd_type fd)
{
	struct lws_pollfd *pfd;

	lwsl_info("%s: DEL fd %d\n", __func__, fd);

	pfd = custom_poll_find_fd(cpcx, fd);
	if (!pfd) {
		lwsl_info("%s: DEL fd %d missing in ext table\n", __func__, fd);
		return 1;
	}

	if (cpcx->count_pollfds > 1)
		*pfd = cpcx->pollfds[cpcx->count_pollfds - 1];

	cpcx->count_pollfds--;

    if (g_ep_mode == eMode::EPOLL)
    {
        int res = epoll_ctl(cpcx->m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
        int eno = errno;
        int uu = 32;
    }

	return 0;
}

static int
custom_poll_change_fd(EpollSet *cpcx, lws_sockfd_type fd,
		     int events_add, int events_remove, unsigned int flags)
{
	struct lws_pollfd *pfd;

	lwsl_info("%s: CHG fd %d, ev_add 0x%x, ev_rem 0x%x\n", __func__, fd,
			events_add, events_remove);

	pfd = custom_poll_find_fd(cpcx, fd);
	if (!pfd)
		return 1;

	pfd->events = (short)((pfd->events & (~events_remove)) | events_add);

    if(g_ep_mode == eMode::EPOLL)// else
    {
        epoll_event evt {};
        evt.data.fd = fd;
        evt.events = pfd->events | EPOLLET;
        int res = epoll_ctl(cpcx->m_epoll_fd, EPOLL_CTL_MOD, evt.data.fd, &evt);
        if(res < 0)
            perror("custom_poll_change_fd failed: ");
        int eno = errno;
        int uu = 32;
    }

	return 0;
}

int init_pt_custom(struct lws_context *cx, void *_loop, int tsi)
{
    lwsl_info("%s", __func__);
	struct pt_eventlibs_custom *priv = (struct pt_eventlibs_custom *)
					     lws_evlib_tsi_to_evlib_pt(cx, tsi);

	/* store the loop we are bound to in our private part of the pt */

	priv->io_loop = (EpollSet *)_loop;

	return 0;
}

int sock_accept_custom(struct lws *wsi)
{
    lwsl_info("%s fd = %d", __func__, lws_get_socket_fd(wsi));
	struct pt_eventlibs_custom *priv = (struct pt_eventlibs_custom *)
						lws_evlib_wsi_to_evlib_pt(wsi);

	return custom_poll_add_fd(priv->io_loop, lws_get_socket_fd(wsi), POLLIN);
}

void io_custom(struct lws *wsi, unsigned int flags)
{
    lwsl_info("%s fd = %d, flags = 0x%x", __func__, lws_get_socket_fd(wsi), flags);
    auto const priv = (pt_eventlibs_custom*) lws_evlib_wsi_to_evlib_pt(wsi);
    //if(g_ep_mode == eMode::POLL)
    {
        int e_add = 0, e_remove = 0;

        if (flags & LWS_EV_START) {
            if (flags & LWS_EV_WRITE)
                e_add |= POLLOUT;

            if (flags & LWS_EV_READ)
                e_add |= POLLIN;
        } else {
            if (flags & LWS_EV_WRITE)
                e_remove |= POLLOUT;

            if (flags & LWS_EV_READ)
                e_remove |= POLLIN;
        }

        custom_poll_change_fd(priv->io_loop, lws_get_socket_fd(wsi),
                    e_add, e_remove, flags);
    }

}

static int
wsi_logical_close_custom(struct lws *wsi)
{
    lwsl_info("%s fd = %d", __func__, lws_get_socket_fd(wsi));
    //if(g_ep_mode == eMode::POLL)
    {
        struct pt_eventlibs_custom *priv = (struct pt_eventlibs_custom *)
                            lws_evlib_wsi_to_evlib_pt(wsi);
        return custom_poll_del_fd(priv->io_loop, lws_get_socket_fd(wsi));
    }
    return 0;
}


BOOST_AUTO_TEST_CASE(with_websockets_wolfssl)
{
    // wolfSSL_CTX_SetIORecv(0,0);
    // auto recv_client = [](struct WOLFSSL* ssl, char* buff, int sz, void* ctx)
    // wolfSSL_SetIORecv();

    // this is a thread contex, processing one event loop
    lws_context_creation_info ws_thread_context_config{};

    // request for custom epoll loop
    static EpollSet g_epoll_set{};
    void *foreign_loops[1] = {&g_epoll_set};
    ws_thread_context_config.foreign_loops = foreign_loops;
    static lws_event_loop_ops event_loop_ops_custom = {};
    event_loop_ops_custom.name                   = "custom",
    event_loop_ops_custom.init_pt                = init_pt_custom,
    event_loop_ops_custom.init_vhost_listen_wsi  = sock_accept_custom,
    event_loop_ops_custom.sock_accept            = sock_accept_custom,
    event_loop_ops_custom.io                     = io_custom,
    event_loop_ops_custom.wsi_logical_close      = wsi_logical_close_custom,
    event_loop_ops_custom.evlib_size_pt          = sizeof(pt_eventlibs_custom);
    static const lws_plugin_evlib_t evlib_custom =
    {
        //.hdr =
        {
            "custom event loop",
            "lws_evlib_plugin",
            LWS_BUILD_HASH,
            LWS_PLUGIN_API_MAGIC
        },
        //.ops	=
        &event_loop_ops_custom};
    ws_thread_context_config.event_lib_custom = &evlib_custom; // bind lws to our custom event
    g_epoll_set.m_epoll_fd = epoll_create1(0);

    // define our connection info
    struct ConnectionInfo
    {
        // more or less required by libwebsockets:
        lws_sorted_usec_list_t sul{}; // schedule connection retry
        lws *wsi{};
        uint16_t retry_count{};
    };

    auto on_my_recv = [](struct lws *wsi, enum lws_callback_reasons reason, void *a_conn_info, void *recv_data, size_t recv_len) -> int
    {
        auto const conn_info = (ConnectionInfo *)a_conn_info;
        switch (reason)
        {
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            BOOST_FAIL("CLIENT_CONNECTION_ERROR: "
                       << (const char *)recv_data);
            goto do_retry;
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            if(g_print_md)
                std::cerr << std::string((char *)recv_data, recv_len) << "\n";
            ++g_msg_recv;
            break;

        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            std::cerr << "established\n";
            // lws_sul_schedule(lws_get_context(wsi), 0, &conn_info->sul_hz,sul_hz_cb, LWS_US_PER_SEC);
            conn_info->wsi = wsi;
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            // lws_sul_cancel(&conn_info->sul_hz);
            goto do_retry;

        default:
            break;
        }
        return lws_callback_http_dummy(wsi, reason, a_conn_info, recv_data, recv_len);
    do_retry:
        //BOOST_FAIL("on_my_recv failed, will not try reconnect");
        g_quit = true;
        // if (lws_retry_sul_schedule_retry_wsi(
        //         wsi,
        //         &conn_info->sul,
        //         connect_client,
        //         &conn_info->retry_count))
        //{
        //     BOOST_FAIL("connection attempts exhausted");
        //     //interrupted = 1;
        // }
        return 0;
    };

    // give libwebsockets our callback for processing inbound data.
    // a callback is called a "protocol"
    const lws_protocols protocols[] = {
        {"lws-minimal-client", on_my_recv, 0, 0, 0, NULL, 0},
        LWS_PROTOCOL_LIST_TERM};
    ws_thread_context_config.protocols = protocols;

    // extensions
    static const lws_extension extensions[] =
        {
            {"permessage-deflate",
             lws_extension_callback_pm_deflate, // does malloc (!)
             "permessage-deflate"
             "; client_no_context_takeover"
             "; client_max_window_bits"},
            {NULL, NULL, NULL}};
    ws_thread_context_config.extensions = extensions;

    // rest of client init
    ws_thread_context_config.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    ws_thread_context_config.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
    ws_thread_context_config.fd_limit_per_thread = 1 + 1 + 1 + g_num_conns;

    // Wolfssl specific, explicit root CA trust
    static const char *const ca_pem_digicert_global_root =
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
    ws_thread_context_config.client_ssl_ca_mem = ca_pem_digicert_global_root;
    ws_thread_context_config.client_ssl_ca_mem_len = (unsigned int)strlen(ca_pem_digicert_global_root);

    // connect
    static lws_context *context = lws_create_context(&ws_thread_context_config);
    BOOST_REQUIRE_MESSAGE(context, "lws_create_context failed");

    auto on_connect_client = [](lws_sorted_usec_list_t *sul)
    {
        ConnectionInfo *mco = lws_container_of(sul, ConnectionInfo, sul);
        lws_client_connect_info i{};

        i.context = context;
        i.port = 443;
        i.address = "fstream.binance.com";
        i.path = "/stream?"
                 "streams="
                 "btcusdt@depth@0ms/"
                 //"btcusdt@bookTicker/"
            //"btcusdt@aggTrade"
            ;
        i.host = i.address;
        i.origin = i.address;
        i.ssl_connection = LCCSCF_USE_SSL | LCCSCF_PRIORITIZE_READS;
        i.protocol = NULL;
        i.local_protocol_name = "lws-minimal-client";
        i.pwsi = &mco->wsi;

        static const uint32_t backoff_ms[] = {1000, 2000, 3000, 4000, 5000};
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
            // if (lws_retry_sul_schedule(context, 0, sul, &retry,
            //             on_connect_client, &mco->retry_count))
            //{
            //     lwsl_info("%s: connection attempts exhausted\n", __func__);
            //     interrupted = 1;
            // }
        }
    };

	signal(SIGINT, sigint_handler);

    std::vector<ConnectionInfo> conns(g_num_conns);

    // schedule the first client connection attempt to happen immediately
    for(auto& conn: conns)
        lws_sul_schedule(context, 0, &conn.sul, on_connect_client, 1);

    using Clock = std::chrono::high_resolution_clock;

    std::vector<CpuTimeStamp> ts;
    ts.reserve(100000000);
    size_t loop_calls = 0;
    auto test_time = std::chrono::seconds(g_test_time_s);
    auto t_bgn = Clock::now();
    auto deadline = t_bgn + test_time;

    if (g_ep_mode == eMode::SPIN)
    {
        int nn = 0;
        while (nn >= 0 && !g_quit && Clock::now() < deadline)
        {
            auto const t0 = rdtscp();
            nn = lws_service(context, -1); // 0: sleep until some event. -1: timeout=0
            auto const t1 = rdtscp();
            ts.push_back(t1-t0);
            if(g_msg_recv == 1){
                t_bgn = Clock::now();
                deadline = t_bgn + test_time;
            }
            ++ loop_calls;
        }
    }
    else
    {
        while(!g_quit && Clock::now() < deadline)
        {
            int n = lws_service_adjust_timeout(context, g_epoll_wait_ms, 0);

            //lwsl_warn("%s: entering poll wait %dms\n", __func__, n);

            EpollSet *const cpcx = &g_epoll_set;

            auto const t0 = rdtscp();
            if (g_ep_mode == eMode::POLL)
                n = poll(cpcx->pollfds, (nfds_t)cpcx->count_pollfds, n);
            else
                n = epoll_wait(cpcx->m_epoll_fd, cpcx->ep_events, 64, n);

            //lwsl_warn("%s: exiting poll ret %d\n", __func__, n);

            if (n <= 0)
                continue;

            {
                int const end = (g_ep_mode == eMode::EPOLL)
                              ? n
                              : cpcx->count_pollfds;
                for (int ii = 0; ii < end; ++ii)
                {
                    pollfd evt2 = {};

                    if(g_ep_mode == eMode::EPOLL)
                    {
                        const epoll_event& evt = cpcx->ep_events[ii];
                        evt2.fd      = evt.data.fd;
                        evt2.events  = evt.events;
                        evt2.revents = evt.events;
                    }
                    else
                    {
                        evt2 = cpcx->pollfds[ii];
                    }

                    lws_sockfd_type fd = evt2.fd;

                    if (!evt2.revents)
                        continue;

                    //lwsl_warn("@@@ exec: fd = %d, evt=%x", fd, evt2.events);

                    int m = lws_service_fd(context, &evt2);

                    auto const t1 = rdtscp();
                    ts.push_back(t1-t0);

                    ++ loop_calls;

                    if(g_msg_recv == 1){
                        t_bgn = Clock::now();
                        deadline = t_bgn + test_time;
                    }

                    // FIXME: this will not work for epoll
                    /* if something closed, retry this slot since may have been
                    * swapped with end fd */
                    if (m && evt2.fd != fd)
                        ii--;

                    if (m < 0)
                        /* lws feels something bad happened, but
                        * the outer application may not care */
                        continue;
                    if (!m)
                    {
                        /* check if it is an fd owned by the
                        * application */
                    }
                }
            }
        }
        std::cerr << "\nep: " << loop_calls << ", cb: " << g_msg_recv << '\n';

        close(g_epoll_set.m_epoll_fd);
    }

    auto t_end = Clock::now();
    std::cerr << format_cc_timings_table(ts, "lws_service(,-1)");
    double hz = double(g_msg_recv) / double((t_end - t_bgn).count() / 1e9);
    std::cerr << "\nep: " << loop_calls << ", cb: " << g_msg_recv << ", " << hz << " hz \n";

    lws_context_destroy(context);
}

// wss://stream.binance.com:9443/ws/bnbbtc@depth
BOOST_AUTO_TEST_CASE(simple_print)
{
    return;
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

        for (;;)
        {
            auto const buf = ws_client.consume_begin();
            std::string data(buf.data, buf.len);
            std::cout << "\n"
                      << data << "\n";
            const size_t consumed = buf.len;
            ws_client.consume_commit(consumed);
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << '\n';
    }
}

BOOST_AUTO_TEST_SUITE_END()
