#pragma once
#include "core/url.h"

// Simple implementation for WebSocket, using blocking semantics.
// This uses boost/beast and was chose for its simplicity and ease of configuration, not performance.
// We should first look into if we can disable encryption for market data web socket.
// It seems likely that when you have multiple websocket connections for the same instrument,
// one of the sockets will receive updates significantly quicker than others.
// If that turns out to be true, we should use an async socket model
// (like the one in ut_asynch_echo test) so that we can have multiple sockets/connections in one
// client for feed arbitrage opportunities.
class WebSocketClient
{
    // Using pimpl because I don't want to contaminate the codede with boost/beast
    // primitives, because it's likely to be replaced by something faster later.
    class Impl;
    Impl* m_impl {};
public:
    explicit WebSocketClient(WebSocketURL const&);
    WebSocketClient() = default;
    ~WebSocketClient();

    void close() noexcept;

    // Sends data, blocking.
    size_t blk_send(void const* data, size_t len);
    size_t blk_send_str(std::string const& str) {return blk_send(str.data(), str.length());}

    // comsume_begin() returns a buffer with data received from the server.
    // once consumed, call consume_commit() to 'remove' the data from the
    // buffer for the next call of consume_begin.
    struct ReceivedData { const char* data; size_t len; };
    ReceivedData consume_begin();
    void consume_commit(size_t);

    WebSocketClient(WebSocketClient const&) = delete;
    WebSocketClient(WebSocketClient&& other) {swap(other);}
    WebSocketClient& operator= (WebSocketClient&& other) {return swap(other);}

private:
    WebSocketClient& swap(WebSocketClient&);
};
