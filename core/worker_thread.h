#pragma once
#include "core/epoll_set.h"
#include "core/i_service.h"
#include "core/i_reconfigurable.h"
#include "core/i_stats_source.h"
#include <thread>
#include <vector>

class WorkerThread
    : public IReconfigurable
    , public IStatsSource
{
public:
    WorkerThread();
    virtual ~WorkerThread();

    void service_add(IService*);
    void service_remove(IService*);
    void service_text_cmd( std::string const& service_name
                         , std::string const& command
                         , std::map<std::string, std::string> args = {});

    void start_separate_thread();
    void signal_stop(bool wait_for_exit = false) noexcept;

private:
    void run_loop_current_thread();
    void run_once_current_thread();

private:
    void run_once_spin();
    void run_once_epoll();

private:
    EpollSet                m_epoll_set;
    IService::eMode         m_mode {IService::eMode::NONE};
    bool volatile           m_quit {false};
    std::vector<IService*>  m_services;
    std::thread             m_thread;
};
