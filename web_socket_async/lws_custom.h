#pragma once

struct lws_plugin_evlib;
struct lws_context;
struct lws;
class EpollSet;

struct LwsCustom
{
    EpollSet* io_loop {};

    static lws_plugin_evlib const& get_vtable_plugin();

private:
    static int  init_pt(lws_context *cx, void *_loop, int tsi);
    static int  sock_accept(lws *wsi);
    static void fdset_io(lws *wsi, unsigned int flags);
    static int  wsi_logical_close(lws *wsi);
};
