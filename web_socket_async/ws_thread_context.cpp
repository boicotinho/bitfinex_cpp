#include "ws_thread_context.h"
#include "lws_custom.h"
#include "core/error.h"
#include <stdint.h>
#include <libwebsockets.h>

namespace
{
} // anon namespace


struct WsThreadContext::Impl
{
    lws_context* const m_lws_context;

    static lws_context* create_from_config(WsThreadContext::Config const& a_cfg)
    {
        lws_context_creation_info lw_cfg {};

        // Hook in support for our epoll-based event loop
        void* tmp_foreign_loops[1] = {a_cfg.epoll_set};
        lw_cfg.foreign_loops = tmp_foreign_loops;
        lw_cfg.event_lib_custom = &LwsCustom::get_vtable_plugin();

        // TODO: translate config
        return lws_create_context(&lw_cfg);
    }

    explicit Impl(WsThreadContext::Config const& a_cfg)
        : m_lws_context(create_from_config(a_cfg))
    {
    }

    ~Impl() { lws_context_destroy(m_lws_context); }

    void service_one(pollfd)
    {

    }

    void service_all()
    {
        const bool wait = false;
        int const res = lws_service(m_lws_context, wait ? 0 : -1);
        if(res)
            THROW_ERRNO("lws_service() failed during service_all()");
    }

    Millis adjust_timeout_for_next_epoll(Millis const a_max_timeout) const
    {
        auto const res = lws_service_adjust_timeout(
                m_lws_context, a_max_timeout.count(), 0);
        return Millis(res);
    }

}; // struct WsThreadContext::Impl

//==============================================================================

WsThreadContext::WsThreadContext(Config const a_cfg)
    : m_impl(new Impl(a_cfg))
{ }

WsThreadContext::~WsThreadContext()
{
    if(m_impl)
        delete m_impl;
}

void WsThreadContext::service_one(pollfd a_pfd)
{
    return m_impl->service_one(a_pfd);
}

void WsThreadContext::service_all()
{
    return m_impl->service_all();
}

Millis WsThreadContext::adjust_timeout_for_next_epoll(Millis a_tmo)
{
    return m_impl->adjust_timeout_for_next_epoll(a_tmo);
}
