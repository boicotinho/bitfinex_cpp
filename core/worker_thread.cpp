#include "worker_thread.h"
#include "core/error.h"
#include "core/time_util.h"
#include "global/constants.h"

WorkerThread::WorkerThread()
{
    m_services.reserve(MAX_SERVICES_PER_THREAD);
}

WorkerThread::~WorkerThread()
{
    signal_stop(true);
}

void WorkerThread::service_add(IService* a_svc)
{
    m_services.push_back(a_svc);
}

void WorkerThread::start_separate_thread()
{
    m_thread = std::thread( &WorkerThread::run_loop_current_thread, this );
}

void WorkerThread::signal_stop(bool wait_for_exit) noexcept
{
    if(wait_for_exit && m_thread.joinable())
        m_thread.join();
    // TODO: add eventfd() to m_epoll_set, then signal that for a clean stop
    m_quit = true;
}

void WorkerThread::run_loop_current_thread()
{
    while(!m_quit)
        run_once_current_thread();
}

void WorkerThread::run_once_current_thread()
{
    switch(m_mode)
    {
        case IService::eMode::SPIN  : return run_once_spin();
        case IService::eMode::EPOLL : return run_once_epoll();
        default : THROW_CERR("Invalid WorkerThread: " << (int)m_mode);
    }
}

void WorkerThread::run_once_spin()
{
    for(IService* svc : m_services)
    {
        svc->on_spin_exec();
        // TODO: upon exception, check svc->get_caps().m_on_error, then
        // either restart the service or kill the server
    }
}

void WorkerThread::run_once_epoll()
{
    m_epoll_set.run_once(MAX_EPOLL_EVENTS_PER_CALL, 5_s);
}
