#pragma once
#include "epoll_event_mask.h"
#include "global/constants.h"
#include "core/error.h"
#include "core/t_uint.h"
#include "core/fast_clock.h"
#include "core/thread_utils.h"
#include "core/string_utils.h"
#include "core/time_util.h"
#include "core/cold_data.h"
#include <chrono>
#include <mutex>
#include <string>
#include <functional>
#include <map>
#include <set>
#include <iosfwd>

struct epoll_event_casted;

// This class is a wrapper for an epoll() or poll() file descriptor.
// Essentially this allows a thread to sleep until some operating system event happens:
//  1) a socket becomes ready to recv/send/accept/error
//  2) a timer expires
//  3) a custom user signal is sent (eventfd file descriptor)
//  4) a file event (create, delete, write) happened for a path registered with inotify
class EpollSet
{
public:
    struct EventArgs;
    struct IEventHandler
    {
        virtual void on_epoll_exec(EventArgs const&, EpollEventMask curr) = 0;
        virtual std::string ep_handler_name() const noexcept; // for logging
        virtual ~IEventHandler() {}
    };
    struct EventArgs
    {
        int             fd;
        EpollEventMask  evt_interest; // libwebsockets needs this
        IEventHandler*  handler;
        bool operator<(EventArgs const& rhs) const {return fd < rhs.fd;}
    };

    // Tell epoll what events to watch for.
    // TODO: think about lifetime management, shared_ptr
    void watch(EventArgs);
    void modwatch(int fd, IEventHandler*, EpollEventMask add, EpollEventMask rem);
    bool unwatch(int fd, IEventHandler const* hint = nullptr) noexcept;
    void unwatch_all_from_handler(IEventHandler const*) noexcept;

    // For logging errors
    void set_name(const std::string& name);
    const std::string& get_name() const;
    void set_log_timings(bool bb) { m_log_timings = bb; }

    // If an EpollSet::Callback throws an exception while executing it's callback,
    // The ExceptionCallback will be called to ask if the event should be kept in epoll.
    // If no ExceptionCallback is set, then the exception will be thrown from RunOnce()
    // instead, and any event pending execution (up to max_events-1) won't get to be
    // executed.
    using ExceptionCallback = std::function<
        bool(EpollSet::IEventHandler&, EventArgs const&, Error const&)
        >;
    void set_exception_callback(ExceptionCallback cb);

    // libwebsockets needs this. Will force, just for the next call,
    // epoll_wait() with a zero timeout.
    void urge_next_run_call() { m_urge_next_call_for_libwebsockets = true; }

    // This is meant to be called by 1 thread only.
    // There should be a 1-to-1 relationship between thread and RunOnce() caller.
    // This is the thread that will actually execute the callbacks for all events.
    // Return number of events processed.
    // If no event happened before timeout, return 0.
    // If a signal took place, return -1.
    // if some other error occurred, FmrError is thrown.
    constexpr static Millis WAIT_FOREVER {-1};
    int run_once(size_t max_events = 256, Nanos timeout = WAIT_FOREVER);
    int try_run_once(size_t max_events = 1) {return run_once(max_events, 0_ns);}

    //int get_native_handle() const {return m_epoll_fd;}

    ~EpollSet();
    EpollSet();
    EpollSet(EpollSet&&)                  = delete;
    EpollSet(const EpollSet&)             = delete;
    EpollSet& operator= (EpollSet&&)      = default;
    EpollSet& operator= (const EpollSet&) = delete;

private:
    using DispatchIndex     = uintv< MAX_EPOLL_EVENTS_PER_CALL >;
    using Clock             = CpuClock;
    using EventSet          = std::set<EventArgs>;
    using Handler2EventsMap = std::map< EpollSet::IEventHandler*, EventSet >;

    struct Cold
    {
        Handler2EventsMap   m_registered_events; // stable storage
        std::mutex          m_mutex; // unwatch_all_from_handler() may be called from another thread
        std::string         m_name; // For logging
        ExceptionCallback   m_exception_callback;
    };

private:
    Cold& cold();
    Cold const & cold() const {return const_cast<EpollSet*>(this)->cold();}
    void close() noexcept;
    void lazy_init(); // we want the correct thread initialize (onload, NUMA)
    void lazy_init_locked();
    bool unwatch_locked(int fd, IEventHandler const* hint = nullptr) noexcept;

    EventArgs* find_fd( int fd
                      , IEventHandler const*         = nullptr
                      , Handler2EventsMap::iterator* = nullptr
                      , EventSet::iterator*          = nullptr
                      ) noexcept; // just for error checking

private:
    int                 m_epoll_fd {-1};
    DispatchIndex       m_dispatching_cur {0};
    bool                m_urge_next_call_for_libwebsockets {false};
    bool                m_log_timings {false};
    epoll_event_casted* m_dispatching_begin {};
    Clock::time_point   m_dispatch_ts {};
};

std::ostream& operator << (std::ostream&, EpollSet::EventArgs const&);
std::ostream& operator << (std::ostream&, EpollSet::IEventHandler const&);
