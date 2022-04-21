#include "ws_thread_context.h"
#include <sys/epoll.h>
#include <sys/poll.h>
#include <libwebsockets.h>

namespace
{

// TODO: promote to core
struct EpollSet
{
    int         m_epoll_fd {-1};
    epoll_event ep_events[64];
    // poll impl, rather than epoll, just for test
    lws_pollfd  pollfds[64];
    int         count_pollfds;
};

// We just tell LWS the size of our struct and it will manage the storage.
struct LwsCustomEventLib// pt_eventlibs_custom
{
    EpollSet *io_loop;
};

} // anon namespace


struct WsThreadContext::Impl
{
    lws_context* const m_lws_context;


    static lws_context* create_from_config(WsThreadContext::Config const& a_cfg)
    {
        lws_context_creation_info cfg {};
        // TODO: translate config
        return lws_create_context(&cfg);
    }

    explicit Impl(WsThreadContext::Config const& a_cfg)
        : m_lws_context()
    {
    }


    ~Impl()
    {

    }

    void service_one(pollfd)
    {

    }

    void service_all()
    {

    }

    Millis adjust_timeout_for_next_epoll(Millis const a_max_timeout)
    {
        //int n = lws_service_adjust_timeout(context, g_epoll_wait_ms, 0);
        return a_max_timeout;
    }

}; // struct WsThreadContext::Impl

//==============================================================================

WsThreadContext::WsThreadContext(Config const a_cfg)
    : m_impl(*new Impl(a_cfg))
{ }

WsThreadContext::~WsThreadContext()
{
    delete &m_impl;
}

void WsThreadContext::service_one(pollfd a_pfd)
{
    return m_impl.service_one(a_pfd);
}

void WsThreadContext::service_all()
{
    return m_impl.service_all();
}

Millis WsThreadContext::adjust_timeout_for_next_epoll(Millis a_tmo)
{
    return m_impl.adjust_timeout_for_next_epoll(a_tmo);
}
