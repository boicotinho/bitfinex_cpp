#include "epoll_set.h"
#include "epoll_event_mask.h"
#include "core/fast_clock.h"
#include "core/log.h"
#include "core/error.h"
#include "core/gcc_utils.h"
#include "core/string_utils.h"
#include "core/x_assert.h"
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/inotify.h>

constexpr std::chrono::milliseconds EpollSet::WAIT_FOREVER;

struct epoll_event_casted // reinterpreted struct epoll_event
{
    EpollEventMask        events;
    EpollSet::EventArgs*  data;

    void mark_end()       {events.mask = 0xffff0000;} // documentation says the higher bits will never be set by epoll_wait
    bool is_end() const   {return events.mask == 0xffff0000;}

    void mark_erased()    {data = nullptr;}
    bool is_erased()const {return !data;}

} __EPOLL_PACKED;

STATIC_ASSERT(sizeof(epoll_event_casted) == sizeof(epoll_event));
STATIC_ASSERT(offsetof(epoll_event_casted, events) == offsetof(epoll_event, events));
STATIC_ASSERT(offsetof(epoll_event_casted, data)   == offsetof(epoll_event, data));

std::string EpollSet::IEventHandler::ep_handler_name() const noexcept
{
    return format_string("%p", (void*)this);
}

std::ostream& operator << (std::ostream& os, EpollSet::EventArgs const& ea)
{
    os << "{fd=" << ea.fd << " @ ";
    if(ea.handler)
        os << *ea.handler;
    else
        os << "<null>";
    os << "}";
    return os;
}

std::ostream& operator << (std::ostream& os, EpollSet::IEventHandler const& eh)
{
    return os << eh.ep_handler_name();
}

EpollSet::EpollSet()
{
    ColdDataRef<EpollSet, EpollSet::Cold>::init(this);
}

EpollSet::~EpollSet()
{
    ColdDataRef<EpollSet, EpollSet::Cold>::release(this);
    close();
}

EpollSet::Cold& EpollSet::cold()
{
    return *ColdDataRef<EpollSet, EpollSet::Cold>::get(this);
}

void EpollSet::set_name(const std::string& name)
{
    cold().m_name = name;
}

const std::string& EpollSet::get_name() const
{
    auto const& name = cold().m_name;
    static std::string s_uninit_name = "<fd=1>";
    return name.empty() ? s_uninit_name : name;
}

void EpollSet::close() noexcept
{
    SLOG_INF("Epoll closing: " << get_name());
    std::unique_lock<std::mutex> lock(cold().m_mutex);
    if(m_epoll_fd < 0)
        return;
    ::close(m_epoll_fd);
    m_epoll_fd = -1;
}

void EpollSet::lazy_init()
{
   if(m_epoll_fd >= 0)
        return;
    std::unique_lock<std::mutex> lock(cold().m_mutex);
    lazy_init_locked();
}

void EpollSet::lazy_init_locked()
{
    if(m_epoll_fd >= 0)
        return;
    m_epoll_fd = epoll_create1(0);
    if(cold().m_name.empty())
        set_name("fd=" + std::to_string(m_epoll_fd));
    SLOG_DBG("EpollSet initialized: " << get_name());
}

void EpollSet::watch( EventArgs const a_evt_args )
{
    if(!a_evt_args.handler)
        THROW_ERROR("Invalid event argument: null");
    if(a_evt_args.fd < 0)
        THROW_ERROR("Invalid event argument: fd=%d", a_evt_args.fd);

    lazy_init();

    EpollEventMask mask = a_evt_args.evt_interest;
    mask.mask |= EPOLLET;

    std::unique_lock<std::mutex> lock(cold().m_mutex);

    if(find_fd(a_evt_args.fd))
        THROW_CERR("fd=" << a_evt_args.fd << " is already being watched by "
            << get_name());

    auto& handler_events = cold().m_registered_events[a_evt_args.handler];
    auto itp = handler_events.insert(a_evt_args);
    EventArgs& new_event_stable_storage = const_cast<EventArgs&>(*itp.first);

    epoll_event ep_evt {};
    ep_evt.events = mask.mask;
    ep_evt.data.ptr = &new_event_stable_storage;

    SLOG_DBG("EpollSet " << get_name() << " watching " << a_evt_args);
    CHECK_SYSCALL(epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, a_evt_args.fd,
                                (epoll_event*)&ep_evt));
}

void EpollSet::modwatch( int            a_fd
                       , IEventHandler* a_handler
                       , EpollEventMask a_mask_add
                       , EpollEventMask a_mask_rem
                       )
{
    if(a_fd < 0)
        THROW_CERR("Invalid event argument: fd=" << a_fd);
    if(!a_handler)
        THROW_CERR("Invalid event argument: null handler");

    lazy_init();

    EventArgs* const evt_args = find_fd(a_fd);
    if(!evt_args)
        THROW_CERR("fd=" << a_fd << " not found in " << get_name());

    evt_args->evt_interest.mask |=  a_mask_add.mask;
    evt_args->evt_interest.mask &= ~a_mask_rem.mask;

    epoll_event evt {};
    evt.data.fd = a_fd;
    evt.events = evt_args->evt_interest.mask;

    SLOG_DBG("EpollSet " << get_name() << " modified " << *evt_args);
    CHECK_SYSCALL(epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, a_fd, &evt));
}

bool EpollSet::unwatch(int a_fd, IEventHandler const* hint) noexcept
{
    EpollSet::Cold& cc = cold();
    std::unique_lock<std::mutex> lock(cc.m_mutex);
    return unwatch_locked(a_fd, hint);
}

bool EpollSet::unwatch_locked(int a_fd, IEventHandler const* hint) noexcept
{
    if(a_fd < 0)
        THROW_CERR("Invalid event argument: fd=" << a_fd);
    Handler2EventsMap::iterator it;
    EventSet::iterator kt;
    if(!find_fd(a_fd, hint, &it, &kt))
    {
        SLOG_ERR("Could not remove fd=" << a_fd
            << " from EpollSet" << get_name());
        return false;
    }
    if(m_epoll_fd >= 0 && epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, a_fd, nullptr))
    {
        SLOG_ERR("EpollSet " << get_name()
            << " failed to remove " << *kt
            << " : " << pretty_errno());
    }
    EventSet& es = it->second;
    es.erase(kt);
    if(es.empty())
        cold().m_registered_events.erase(it);
    return true;
}

void EpollSet::unwatch_all_from_handler(IEventHandler const* a_handler) noexcept
{
    if(!a_handler)
        THROW_ERROR("Invalid callback argument");

    EpollSet::Cold& cc = cold();

    std::unique_lock<std::mutex> lock(cc.m_mutex);

    auto it= cc.m_registered_events.find(const_cast<IEventHandler*>(a_handler));
    if(it == cc.m_registered_events.end())
    {
        SLOG_WAR("EpollSet (" << get_name()
            << ") unwatch_all_from_handler: " << a_handler->ep_handler_name()
            << " not found");
        return;
    }

    if(m_epoll_fd >= 0)
    {
        std::set<EventArgs> const& arg_set = it->second;

        for(EventArgs const& ea: arg_set)
        {
            if(epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, ea.fd, nullptr))
            {
                SLOG_WAR("EpollSet " << get_name()
                    << " failed to remove " << ea
                    << " (" << a_handler->ep_handler_name()
                    << "): " << pretty_errno());
            }
        }
    }

    // If we are currently dispatching epoll_pwait callbacks...
    auto const bgn = m_dispatching_begin;
    auto const end = bgn + MAX_EPOLL_EVENTS_PER_CALL;
    for (epoll_event_casted* ee = bgn; ee < end && ee->is_end(); ++ee)
    {
        if(ee->data->handler == a_handler)
        {
            SLOG_DBG("Removing epoll event already present in the same epoll ("
                << get_name() << ") return batch: ["
                << a_handler->ep_handler_name() << "], mask: " << ee->events);
            ee->mark_erased();
        }
    }

    cc.m_registered_events.erase(it);
}

EpollSet::EventArgs*
EpollSet::find_fd( int                          const a_fd
                 , IEventHandler const*         const a_handler_hint
                 , Handler2EventsMap::iterator* const a_p_it
                 , EventSet::iterator*          const a_p_kt
                 ) noexcept
{
    EventArgs const fd_search_key {a_fd, EpollEventMask(),nullptr};
    auto& events = cold().m_registered_events;
    if(a_handler_hint)
    {
        auto it = events.find(const_cast<IEventHandler*>(a_handler_hint));
        if(it == events.end())
            return nullptr;
        std::set<EventArgs>& arg_set = it->second;
        auto kt = arg_set.find(fd_search_key);
        if(kt != arg_set.end())
        {
            if(a_p_it)
                *a_p_it = it;
            if(a_p_kt)
                *a_p_kt = kt;
            return &const_cast<EventArgs&>(*kt);
        }
        return nullptr;
    }
    for(auto it = events.begin(); it != events.end(); ++it)
    {
        std::set<EventArgs>& arg_set = it->second;
        auto kt = arg_set.find(fd_search_key);
        if(kt != arg_set.end())
        {
            if(a_p_it)
                *a_p_it = it;
            if(a_p_kt)
                *a_p_kt = kt;
            return &const_cast<EventArgs&>(*kt);
        }
    }
    return nullptr;
}

void EpollSet::set_exception_callback(ExceptionCallback cb)
{
    cold().m_exception_callback = std::move(cb);
}

int EpollSet::run_once(size_t const a_max_events_unsanitized, Nanos a_timeout)
{
    size_t const a_max_events = std::min( a_max_events_unsanitized,
                                          MAX_EPOLL_EVENTS_PER_CALL);
    if(UNLIKELY(m_urge_next_call_for_libwebsockets))
    {
        a_timeout = 0_s;
        m_urge_next_call_for_libwebsockets = false;
    }

    // rounding ns -> ms : +microseconds(500)
    using namespace std::chrono;
    const Millis timeout_ms = duration_cast<Millis>(a_timeout);
    epoll_event_casted resulting_signaled_events [MAX_EPOLL_EVENTS_PER_CALL];

    Clock::time_point t0 {};
    if(m_log_timings)
    {
        t0 = Clock::now();
        if(!m_dispatch_ts)
            m_dispatch_ts = t0;
        SLOG_DBG("EpollSet " << get_name() << " epoll_pwait()...");
    }
    // epoll_pwait2() offers nanosecond precision for timeouts, but is only available from kernel 5.11
    const sigset_t* const sig_set = nullptr; // signals to block and prevent from waking up epoll_pwait
    const int res = epoll_pwait( m_epoll_fd
                               , (epoll_event*)resulting_signaled_events
                               , a_max_events
                               , timeout_ms.count()
                               , sig_set
                               );
    if(m_log_timings)
    {
        auto const time_outside = t0 - m_dispatch_ts;
        m_dispatch_ts = Clock::now();
        auto const time_inside = m_dispatch_ts - t0;
        SLOG_DBG("EpollSet " << get_name() << " epoll_pwait() "
                    << time_outside << " outside, "
                    << time_inside << " waiting, "
                    << "wake_res=" << res);
    }
    if(0 == res)
        return 0; // Time-out expired.
    if(res < 0) {
        if(EINTR == errno){
            return -1; // Interrupted by a signal not in sig_set, e.g.: SIGINT, SIGKILL, SIGUSR1 etc
        }
        // Real error
        THROW_ERRNO("epoll_pwait(epfd=%d, _, num_events=%zu, timeout_ms=%ld, _) %s",
            m_epoll_fd, a_max_events, timeout_ms.count(), get_name().c_str());
    }

    // Mark begin/end of iteration. Needed if events get removed from epoll
    // while processing a callback in the loop below.
    // It's fine that it points to the stack - the callbacks are called from
    // the same thread and the pointers will remain good while any callback is
    // taking place.
    resulting_signaled_events[res].mark_end();
    m_dispatching_begin = resulting_signaled_events;
    m_dispatching_cur = 0;

    // A number of events occurred. Let's invoke their callbacks
    for(int ee = 0; ee < res; ++ee)
    {
        const epoll_event_casted& ep_evt = resulting_signaled_events[ee];
        // e.g. event[3] erased by event[1] callback, same epoll return batch
        if(ep_evt.is_erased())
            continue;
        EpollSet::EventArgs& event_obj = *ep_evt.data;
        try
        {
            event_obj.handler->on_epoll_exec(event_obj, ep_evt.events);
        }
        catch(Error& ex)
        {
            SLOG_ERR("EpollSet::IEventHandler exception while executing "
                    << event_obj << ", Epoll: " << get_name()
                    << ", error: " << ex.what());
            const bool keep = cold().m_exception_callback(
                    *event_obj.handler, event_obj, ex);
            if(!keep)
                unwatch_locked(event_obj.fd, event_obj.handler);
        }
        m_dispatching_cur++;
    }
    m_dispatching_cur = 0;
    m_dispatching_begin = nullptr;
    return res; // number of events that took place
}
