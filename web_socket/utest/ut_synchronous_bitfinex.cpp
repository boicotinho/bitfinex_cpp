#include <boost/test/unit_test.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

#include <atomic>
#include <thread>
#include <x86intrin.h>

BOOST_AUTO_TEST_SUITE(wss)

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

#define COMPILER_BARRIER()  __asm__ __volatile("")

uint64_t rdtscp()
{
    uint32_t aux;
    COMPILER_BARRIER();
    uint64_t res = __rdtscp(&aux);
    COMPILER_BARRIER();
    return res;
}

class TestClient
{
    using WebSocket = websocket::stream<beast::ssl_stream<tcp::socket>>;
    net::io_context m_ioc;
    ssl::context    m_ctx       {ssl::context::tlsv12_client};
    tcp::resolver   m_resolver  {m_ioc};
    WebSocket       m_ws        {m_ioc, m_ctx};
public:
    TestClient( const std::string& host,
                const std::string& port,
                const std::string& hshk )
    {
        // Look up the domain name
        auto const results = m_resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        net::connect(m_ws.next_layer().next_layer(), results.begin(), results.end());

        // Perform the SSL handshake
        m_ws.next_layer().handshake(ssl::stream_base::client);

        // Set a decorator to change the User-Agent of the handshake
        m_ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-coro");
            }));

        // Perform the websocket handshake
        m_ws.handshake(host, hshk);
    }

    ~TestClient()
    {
        beast::error_code ec;
        m_ws.close(websocket::close_code::normal, ec); // none, normal
    }

    WebSocket& ws() {return m_ws;}
};

using CountDownVal = std::atomic<int>;

struct Args
{
    Args() = default;
    Args(const std::string& jrpc_req, CountDownVal* cnt) : req(jrpc_req), p_countdown(cnt) {}
    bool print       {false};
    bool timestamps  {true};
    size_t num_recvs {2};
    std::string host {"api-pub.bitfinex.com"};
    std::string port {"443"};
    std::string hshk {"/ws/2"};
    std::string req  {R"({ "event": "subscribe", "channel": "book", "symbol": "tBTCUSD" })"};
    // Sync multi-threaded startup
    CountDownVal* p_countdown {};
    // Work Result
    std::vector<uint64_t> ts;
    beast::flat_buffer result_buffer;
};

void RunTest(Args& args)
{
    TestClient client(args.host, args.port, args.hshk);

    if(args.p_countdown)
    {
        -- *args.p_countdown;
        while(args.p_countdown->load())
            _mm_pause();
    }

    // Our message in this case should be stringified JSON-RPC request
    client.ws().write(net::buffer(args.req));

    // This buffer will hold the incoming message
    args.result_buffer.reserve(100000 + args.num_recvs * 64);

    args.ts.reserve(args.num_recvs);

    // Read a message into our buffer
    ssize_t rd = 0;

    for(size_t ii = 0; ii < args.num_recvs; ++ii)
    {
        rd = client.ws().read(args.result_buffer);
        const auto now = rdtscp();
        if(rd <= 0)
        {
            break;
        }
        if(args.print)
        {
            auto const segment = args.result_buffer.cdata();
                std::cout << now << " rd=" << rd << "; '" << beast::make_printable(segment) << "'\n";
            args.result_buffer.consume(segment.size());
        }
        if(rd < 200 && args.timestamps)
        {
            args.ts.push_back(now);
        }
    }
    //std::cout << beast::make_printable(args.result_buffer.data()) << std::endl;
}

BOOST_AUTO_TEST_CASE(synchronous_bitfinex_P0)
{
    Args args(R"({ "event": "subscribe", "channel": "book", "symbol": "tBTCUSD" })", nullptr);
    args.print = true;
    RunTest(args);
}

BOOST_AUTO_TEST_CASE(synchronous_bitfinex_R0)
{
    Args args(R"({ "event": "subscribe", "channel": "book", "prec": "R0", "symbol": "tBTCUSD" })", nullptr);
    args.print = true;
    RunTest(args);
}

BOOST_AUTO_TEST_CASE(synchronous_bitfinex_both)
{
    Args args(R"({ "event": "subscribe", "channel": "book", "prec": "R0", "symbol": "tBTCUSD", "subId":"888" }{ "event": "subscribe", "channel": "book", "symbol": "tBTCUSD", "subId":"999" })", nullptr);
    args.print = true;
    RunTest(args);
}

BOOST_AUTO_TEST_CASE(R0_P0_perf_compare)
{
    CountDownVal ready_countdown {2};

    Args args_p0(R"({ "event": "subscribe", "channel": "book", "symbol": "tBTCUSD" })", &ready_countdown);
    Args args_r0(R"({ "event": "subscribe", "channel": "book", "prec": "R0", "symbol": "tBTCUSD" })", &ready_countdown);

    // args_p0.print = true;
    // args_r0.print = true;
    // args_p0.timestamps = false;
    // args_r0.timestamps = false;

    std::thread thread_p0(RunTest, std::ref<Args>(args_p0));
    std::thread thread_r0(RunTest, std::ref<Args>(args_r0));

    thread_p0.join();
    thread_r0.join();
    std::vector<int64_t> diff; // when is feed r0 faster than p0?

    const size_t sz = std::min(args_p0.ts.size(), args_r0.ts.size());
    for(size_t ii = 0; ii < sz; ++ii)
    {
        const int64_t dd = args_r0.ts[ii] - args_p0.ts[ii];
        std::cout << "Diff: " << dd << "\n";
        diff.push_back(dd);
    }
}


BOOST_AUTO_TEST_SUITE_END()
