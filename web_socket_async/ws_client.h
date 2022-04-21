#pragma once
#include "core/url.h"
#include "core/throttle.h"
#include "core/str_view.h"
#include <functional>
#include <memory>
#include <utility>

class WsThreadContext;

// For sending orders we gonna need another lib or roll our own libwebsockets
class WsClient // lws, lws_sorted_usec_list_t
{
public:
    struct ICallback
    {
        // User must return the number of bytes consumed.
        // Unconsumed data will be repeated in next callback invocation.
        size_t on_recv(StrView);

        // Reported when we get disconnected but still have some attemts left.
        void on_connection_unstable();

        // Called when either socket failed to connect the first time, or later
        // during the day after failing 3 retries (depending on configuration).
        void on_connection_closed();
    };
    using ICallbackPtr = std::shared_ptr<ICallback>;

    // TODO: review shared_ptr: sockets, epoll, services, worker threads

    using OnWrite = std::function<size_t(void* buffer, size_t max_len)>;
    void async_send(OnWrite); // lws_ss_request_tx

    void async_connect( WsThreadContext&
                      , ICallbackPtr
                      , WebSocketURL const&);

    void close() noexcept;

    WsClient() = default;
    WsClient(WsClient&& other) {swp(other);}
    WsClient& operator=(WsClient&& other) {return swp(other);}
    ~WsClient() {close();}

private:
    struct Impl;
    WsClient& swp(WsClient& other);

private:
    Impl* m_impl {};
    //struct lws;
    //lws* m_lws {};
};
