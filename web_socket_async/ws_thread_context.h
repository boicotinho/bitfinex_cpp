#pragma once
#include "core/time_util.h"
#include "core/throttle.h"
#include <vector>
#include <stdint.h>
#include <sys/poll.h> // struct pollfd is needed by libwebsockets

// This manages a set of sockets which are meant to be processed
// by 1 epoll set running from 1 thread.
class WsThreadContext // lws_context
{
public:
    struct Config;
    explicit WsThreadContext(Config);
    WsThreadContext() = default;
    ~WsThreadContext();

    void service_one(pollfd); // for epoll
    void service_all();       // for spin
    Millis adjust_timeout_for_next_epoll(Millis); // required by libwebsockets

public:
    struct Config
    {
        Throttle::Limit     send_throttle_limit     {10, 1_s};
        size_t              max_sockets_per_thread  {1024};
        Millis              heartbeat_interval      {1_mins}; // a.k.a. ping
        Millis              heartbeat_hangup        {10_mins};
        std::vector<Micros> conn_backoff_table      {1_s, 2_s, 4_s, 5_s};
        std::vector<Micros> conn_conceal_table      {conn_backoff_table};
    };

private:
    friend class WsClient;
    struct Impl;

private:
    Impl& m_impl;
};