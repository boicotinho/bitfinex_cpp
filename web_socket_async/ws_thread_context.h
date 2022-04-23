#pragma once
#include "core/time_util.h"
#include "core/throttle.h"
#include "core/epoll_set.h"
#include <vector>
#include <utility>
#include <stdint.h>
#include <sys/poll.h> // struct pollfd is needed by libwebsockets

// This manages a set of sockets which are meant to be processed
// by 1 epoll set running from 1 thread.
class WsThreadContext // lws_context
{
public:
    struct Config
    {
        EpollSet*           epoll_set               {nullptr};
        Throttle::Limit     send_throttle_limit     {10, 1_s};
        size_t              max_sockets_per_thread  {1024};
        Millis              heartbeat_interval      {1_mins}; // a.k.a. ping
        Millis              heartbeat_hangup        {10_mins};
        std::vector<Micros> conn_backoff_table      {1_s, 2_s, 4_s, 5_s};
        std::vector<Micros> conn_conceal_table      {conn_backoff_table};
    };

    void service_one(pollfd); // for epoll
    void service_all();       // for spin
    Millis adjust_timeout_for_next_epoll(Millis); // required by libwebsockets

    WsThreadContext() = default;
    explicit WsThreadContext(Config);
    WsThreadContext(WsThreadContext&& other) {swp(other);}
    WsThreadContext& operator=(WsThreadContext&& other) {return swp(other);}
    ~WsThreadContext();
    explicit operator bool() const {return !!m_impl;}

private:
    friend class WsClient;
    struct Impl;
    WsThreadContext& swp(WsThreadContext& other)
        {std::swap(m_impl, other.m_impl); return *this;}

private:
    Impl* m_impl {};
};
