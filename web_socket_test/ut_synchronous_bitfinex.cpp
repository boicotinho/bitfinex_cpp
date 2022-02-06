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

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

BOOST_AUTO_TEST_SUITE(wss)

BOOST_AUTO_TEST_CASE(synchronous_bitfinex)
{
    auto const host = "api-pub.bitfinex.com";
    auto const port = "443";
    auto const rpcJson = R"({ "event": "subscribe", "channel": "book", "symbol": "tBTCUSD" })";
    auto const hshk = "/ws/2";

    // The io_context is required for all I/O
    net::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    // This holds the root certificate used for verification
    //load_root_certificates(ctx);

    // These objects perform our I/O
    tcp::resolver resolver{ioc};
    websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    net::connect(ws.next_layer().next_layer(), results.begin(), results.end());

    // Perform the SSL handshake
    ws.next_layer().handshake(ssl::stream_base::client);

    // Set a decorator to change the User-Agent of the handshake
    ws.set_option(websocket::stream_base::decorator(
        [](websocket::request_type& req)
        {
            req.set(http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-coro");
        }));

    // Perform the websocket handshake
    ws.handshake(host, hshk);

    // Our message in this case should be stringified JSON-RPC request
    ws.write(net::buffer(std::string(rpcJson)));

    // This buffer will hold the incoming message
    beast::flat_buffer buffer;

    // Read a message into our buffer
    ssize_t rd = 0;

    for(int ii = 0; ii < 10; ++ii)
    {
        rd = ws.read(buffer);
        if(rd <= 0)
        {
            break;
        }
        auto const segment = buffer.cdata();
        std::cout << "rd=" << rd << "; '" << beast::make_printable(segment) << "'\n\n";
        buffer.consume(segment.size());
    }

    // Close the WebSocket connection
    beast::error_code ec;
    ws.close(websocket::close_code::normal, ec); // none, normal
    //std::cout << "rd=" << rd << "; " <<  beast::make_printable(buffer.data()) << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
