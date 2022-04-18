#pragma once
#include "core/config.h"
#include <vector>
#include <array>
#include <set>
#include <memory>
#include <string>
#include <chrono>
#include <type_traits>
#include <stdexcept>
#include <unordered_map>
#include <map>
#include <stdint.h>

using fd_t = int;
enum { MAX_FD_PER_EPOLL = 1024 };


class EpollSet
{
public:
    enum eEvent { NONE = 0, READ = 1, WRITE = 2, READ_WRITE = READ | WRITE };

    struct pollfd
    {
        fd_t     fd;
        uint16_t events;
        uint16_t reserved;
    };

    struct Callback
    {
        virtual void on_epoll_exec(pollfd const&, eEvent) = 0;
    };
    //using CallbackSPtr = std::shared_ptr<Callback>;

    void watch(int fd, eEvent, Callback*);
    void unwatch(int fd, eEvent, Callback*); // if both events end up unwatched, the fd is removed from epoll
    void unwatch_by_callback(Callback const*) noexcept;

private:
    struct Entry
    {
        struct pollfd   m_pollfd;
        Callback*       m_callback;
    };

    struct Cold
    {
        std::unordered_map<fd_t, size_t>         m_fd_2_index;
        std::array<pollfd , MAX_FD_PER_EPOLL>    m_pollfd_table;
        std::array<Callback* , MAX_FD_PER_EPOLL> m_callbacks;
    };

private:
    int  m_fd {-1};
    std::unique_ptr<Cold> m_cold; // m_pollfd_table is needed by poll, not epoll
};


//using IEpollCallback = EpollSet::Callback;

struct ISpinCallback
{
    virtual void on_spin_exec() = 0;
};

using Any = double; // Need c++17 for std::any. Alternatively, boost

struct IStatsSource
{
    using StatName = std::string;

    virtual std::vector<StatName>
        stats_enum() const { return {}; }

    virtual void
        stats_read_thread_safe(std::vector<Any>& out) const {}
};


struct IReconfigurable
{
    virtual void configure(Config const&, std::string const& section={}) {}
    virtual bool reconfigure(Config const& new_cfg) noexcept {return false;}
};

struct IService
    : public IReconfigurable    // virtual reconfigure(Config new_config)
    , public IStatsSource       // virtual stats_read_thread_safe()
    , public EpollSet::Callback // virtual on_epoll_exec()
    , public ISpinCallback      // virtual on_spin_exec()
{
public:
    enum class eOnError   { TERMINATE_PROCESS, RESTART_SERVICE };
    enum class eMode:char { NONE, SPIN = 1, EPOLL = 2, SPIN_AND_EPOLL = 3 };
    using ServiceNames = std::set<std::string>;
    struct Caps
    {
        std::string  m_name                {"Unamed"};
        eMode        m_supported_modes     {eMode::NONE};
        eOnError     m_on_error            {eOnError::TERMINATE_PROCESS};
        ServiceNames m_services_init_before;
        ServiceNames m_services_init_after;
    };
public:
    virtual ~IService() {}
    virtual Caps get_caps() const = 0;
    virtual void on_start(WorkerThread* parent) {}
    virtual void on_stop(WorkerThread* parent) {}
};

class WorkerThread
    : public IReconfigurable
    , public IStatsSource
{
public:
    void service_add(IService*);
    void service_remove(IService*);
    void service_text_cmd( std::string const& service_name
                         , std::string const& command
                         , std::map<std::string, std::string> args = {});

    void run_once_current_thread();
    void run_loop_current_thread();
    void start_separate_thread();
    void signal_stop(bool wait_for_exit = false) noexcept;

    virtual ~WorkerThread();
private:
    EpollSet                m_epoll_set;
    IService::eMode         m_mode {IService::eMode::NONE};
    std::vector<IService*>  m_services;
};
