#pragma once
#include "core/i_reconfigurable.h"
#include "core/i_stats_source.h"
#include "core/epoll_set.h"

class WorkerThread;

struct ISpinCallback
{
    virtual void on_spin_exec() = 0;
};

struct IService
    : public IReconfigurable         // virtual reconfigure(Config new_config)
    , public IStatsSource            // virtual stats_read_thread_safe()
    , public EpollSet::IEventHandler // virtual on_epoll_exec()
    , public ISpinCallback           // virtual on_spin_exec()
{
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

    virtual ~IService() {}
    virtual Caps get_caps() const = 0;
    virtual void on_start(WorkerThread* parent) {}
    virtual void on_stop(WorkerThread* parent) {}
};
