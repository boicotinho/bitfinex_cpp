#define USE_REAL_CERTIFICATES 0

#include <boost/version.hpp> // 1.76

#if (USE_REAL_CERTIFICATES == 1)
    // #include <openssl/conf.h> // check if installed? sudo yum install -y openssl-devel
    // /home/fabio/dev/certify/include/ : git clone --recursive https://github.com/djarek/certify
    #include <boost/certify/extensions.hpp>
    #include <boost/certify/https_verification.hpp>
#elif (USE_REAL_CERTIFICATES == 2)
    #include "root_certificates.hpp"
#endif

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Sends a WebSocket message and prints the response
class session : public std::enable_shared_from_this<session>
{
    tcp::resolver resolver_;
    websocket::stream<
        beast::ssl_stream<beast::tcp_stream>> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string text_;
    std::string handshake_;

public:
    // Resolver and socket require an io_context
    explicit
    session(net::io_context& ioc, ssl::context& ctx)
        : resolver_(net::make_strand(ioc))
        , ws_(net::make_strand(ioc), ctx)
    {
        std::cerr << __FUNCTION__ << "\n";
    }

    // Start the asynchronous operation
    void
    run(
        char const* host,
        char const* port,
        char const* text,
        char const* handshake)
    {
        std::cerr << __FUNCTION__ << "\n";
        // Save these for later
        host_ = host;
        text_ = text;
        handshake_ = handshake;

        // Look up the domain name
        resolver_.async_resolve(
            host,
            port,
            beast::bind_front_handler(
                &session::on_resolve,
                shared_from_this()));
    }

    void
    on_resolve(
        beast::error_code ec,
        tcp::resolver::results_type results)
    {
        std::cerr << __FUNCTION__ << "\n";
        if(ec)
            return fail(ec, "resolve");

        // Set a timeout on the operation
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(ws_).async_connect(
            results,
            beast::bind_front_handler(
                &session::on_connect,
                shared_from_this()));
    }

    void
    on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
    {
        std::cerr << __FUNCTION__ << "\n";
        if(ec)
            return fail(ec, "connect");

        // Set a timeout on the operation
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(
                ws_.next_layer().native_handle(),
                host_.c_str()))
        {
            ec = beast::error_code(static_cast<int>(::ERR_get_error()),
                net::error::get_ssl_category());
            return fail(ec, "connect");
        }

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host_ += ':' + std::to_string(ep.port());

        // Perform the SSL handshake
        ws_.next_layer().async_handshake(
            ssl::stream_base::client,
            beast::bind_front_handler(
                &session::on_ssl_handshake,
                shared_from_this()));
    }

    void
    on_ssl_handshake(beast::error_code ec)
    {
        std::cerr << __FUNCTION__ << "\n";
        if(ec)
            return fail(ec, "ssl_handshake");

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(ws_).expires_never();

        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-async-ssl");
            }));

        // Perform the websocket handshake
        ws_.async_handshake(host_, handshake_,
            beast::bind_front_handler(
                &session::on_handshake,
                shared_from_this()));
    }

    void
    on_handshake(beast::error_code ec)
    {
        std::cerr << __FUNCTION__ << "\n";
        if(ec)
            return fail(ec, "handshake");

        // Send the message
        ws_.async_write(
            net::buffer(text_),
            beast::bind_front_handler(
                &session::on_write,
                shared_from_this()));
    }

    void
    on_write(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        std::cerr << __FUNCTION__ << "\n";
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        // Read a message into our buffer
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        std::cerr << __FUNCTION__ << "\n";
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        // Close the WebSocket connection
        ws_.async_close(websocket::close_code::normal,
            beast::bind_front_handler(
                &session::on_close,
                shared_from_this()));
    }

    void
    on_close(beast::error_code ec)
    {
        std::cerr << __FUNCTION__ << "\n";

        if(ec)
        {
            std::cout << "RECEIVED, closed disgracefully:\n";
            std::cout << beast::make_printable(buffer_.data()) << std::endl;
            return fail(ec, "close");
        }

        // If we get here then the connection is closed gracefully
        std::cout << "RECEIVED, closed gracefully:\n";

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }
};

//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    // ws://websocket-echo.com // ports 443 and 80
    auto host = "api-pub.bitfinex.com"; // /ws/2
    auto port = "443";
    auto hshk = "/ws/2"; // "/ws/2" // '/' for echo server
    auto text = R"({ "event": "subscribe", "channel": "book", "symbol": "tBTCUSD" })"; // JSON-RPC request

    // auto host = "websocket-echo.com";
    // auto port = "443";
    // auto hshk = "/";
    // auto text = "MyWebSocketTest";

    // Check command line arguments.
    //if(argc != 4)
    //{
    //    std::cerr <<
    //        "Usage: websocket-client-async-ssl <host> <port> <text>\n" <<
    //        "Example:\n" <<
    //        "    websocket-client-async-ssl echo.websocket.org 443 \"Hello, world!\"\n";
    //    return EXIT_FAILURE;
    //}

    if(argc > 1)
        host = argv[1];
    if(argc > 2)
        port = argv[2];
    if(argc > 3)
        hshk = argv[3];
    if(argc > 4)
        text = argv[4];

    // The io_context is required for all I/O
    net::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client}; // tlsv12_client

    // This holds the root certificate used for verification
#if (USE_REAL_CERTIFICATES == 0)
#elif (USE_REAL_CERTIFICATES == 1)
    ctx.set_verify_mode(ssl::context::verify_peer); // verify_none
    boost::certify::enable_native_https_server_verification(ctx);
#elif (USE_REAL_CERTIFICATES == 2)
    load_root_certificates(ctx);
    // ctx.set_verify_mode(ssl::context::verify_peer);
#else
    #error "Invalid USE_REAL_CERTIFICATES"
#endif

    // Launch the asynchronous operation
    std::make_shared<session>(ioc, ctx)->run(host, port, text, hshk);

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();

    return EXIT_SUCCESS;
}
