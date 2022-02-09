#include "web_socket_client.h"
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
#include <algorithm>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class WebSocketClient::Impl
{
    using WebSocket0    = websocket::stream<tcp::socket>; // no encryption (not sure if we can disable at the exchange)
    using WebSocket1    = websocket::stream<beast::ssl_stream<tcp::socket>>;
    net::io_context     m_ioc;
    ssl::context        m_ctx       {ssl::context::tlsv12_client};
    tcp::resolver       m_resolver  {m_ioc};
    WebSocket1          m_ws        {m_ioc, m_ctx};
    bool                m_open      {false};
    beast::flat_buffer  m_recv_buf;
public:
    Impl(WebSocketURL const& a_url)
    {
        m_open = true;
        m_recv_buf.reserve(65536);
        // Look up the domain name
        auto const results = m_resolver.resolve(a_url.host, a_url.port);

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
        m_ws.handshake(a_url.host, a_url.hand_shake);
    }

    ~Impl() { close(); }

    void close() noexcept
    {
        if(!m_open)
            return;
        beast::error_code ec;
        m_ws.close(websocket::close_code::normal, ec); // none, normal
        m_open = false;
    }

    size_t blk_send(void const* data, size_t len)
    {
        return m_ws.write(net::buffer(data, len));
    }

    ReceivedData consume_begin()
    {
        const size_t num_recvd = m_ws.read(m_recv_buf);
        (void) num_recvd;
        auto const segment = m_recv_buf.cdata();
        return {(const char*)segment.data(), segment.size()};
    }

    void consume_commit(size_t len)
    {
        m_recv_buf.consume(len);
    }
};



WebSocketClient::WebSocketClient(WebSocketURL const& url)
    : m_impl (new WebSocketClient::Impl(url))
{
}

WebSocketClient::~WebSocketClient()
{
    if(m_impl)
    {
        this->close();
        delete m_impl;
        m_impl = nullptr;
    }
}

void WebSocketClient::close() noexcept
{
    if(!m_impl)
        return;
    m_impl->close();
}

WebSocketClient& WebSocketClient::swap(WebSocketClient& other)
{
    std::swap(this->m_impl, other.m_impl);
    return *this;
}

size_t WebSocketClient::blk_send(void const* data, size_t len)
{
    if(!m_impl)
        return 0;
    return m_impl->blk_send(data, len);
}

WebSocketClient::ReceivedData
WebSocketClient::consume_begin()
{
    if(!m_impl)
        return {};
    return m_impl->consume_begin();
}

void WebSocketClient::consume_commit(size_t len)
{
    if(!m_impl)
        return;
    return m_impl->consume_commit(len);
}
