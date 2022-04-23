#include "lws_custom.h"
#include "core/epoll_set.h"
#include <libwebsockets.h>

lws_plugin_evlib const& LwsCustom::get_vtable_plugin()
{
    static lws_plugin_evlib g_plugin = []()
    {
        lws_event_loop_ops evt_loop_ops {};
        evt_loop_ops.name                   = "custom",
        evt_loop_ops.init_pt                = LwsCustom::init_pt,
        evt_loop_ops.init_vhost_listen_wsi  = LwsCustom::sock_accept,
        evt_loop_ops.sock_accept            = LwsCustom::sock_accept,
        evt_loop_ops.io                     = LwsCustom::fdset_io,
        evt_loop_ops.wsi_logical_close      = LwsCustom::wsi_logical_close,
        evt_loop_ops.evlib_size_pt          = sizeof(LwsCustom);

        lws_plugin_evlib_t plugin {};
        plugin.hdr.name           = "custom event loop";
        plugin.hdr._class         = "lws_evlib_plugin";
        plugin.hdr.lws_build_hash = LWS_BUILD_HASH;
        plugin.hdr.api_magic      = LWS_PLUGIN_API_MAGIC;
        plugin.ops                = &evt_loop_ops;
        return plugin;
    }();
    return g_plugin;
}

int LwsCustom::init_pt(lws_context* cx, void* _loop, int tsi)
{
    lwsl_info("%s", __func__);
    auto priv = (LwsCustom*) lws_evlib_tsi_to_evlib_pt(cx, tsi);
    priv->io_loop = (EpollSet*) _loop;
    return 0;
}

int LwsCustom::sock_accept(lws *wsi)
{
    int const fd = lws_get_socket_fd(wsi);
    LwsCustom& priv = *(LwsCustom*) lws_evlib_wsi_to_evlib_pt(wsi);
    #pragma GCC diagnostic push
   // #pragma GCC diagnostic ignored "-Wdeprecated"
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    // error: ‘const lws_protocols* lws_protocol_get(lws*)’ is deprecated [-Werror=deprecated-declarations]
    const lws_protocols& proto = *lws_protocol_get(wsi);
    #pragma GCC diagnostic pop

    lwsl_info("%s fd = %d", __func__, fd);

    proto.callback;
    proto.user;
    //priv.io_loop->watch(fd, service_on(myfd) , EPOLLIN);
    return 0;
}

void LwsCustom::fdset_io(lws *wsi, unsigned int flags)
{
}

int  LwsCustom::wsi_logical_close(lws *wsi)
{
    return 0;
}
