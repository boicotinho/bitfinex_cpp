#pragma once
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <x86intrin.h>

// No-Op mutex for thread-unsafe feed traits
struct NullMutex
{
    bool try_lock() const {return true;}
    void lock() const {}
    void unlock() const {}
};

// Busy spin-wait mutex for thread-safe feed traits
struct SpinMutex
{
    bool try_lock() const
        {
        return !flag.test_and_set(std::memory_order_acquire);
        }
    void lock() const
        {
        while(flag.test_and_set(std::memory_order_acquire))
            _mm_pause();
        }
    void unlock() const
        {
        flag.clear(std::memory_order_release);
        }
private:
    mutable std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

using SleepMutex = std::mutex;

class Semaphore
 {
    mutable std::mutex              m_mtx;
    mutable std::condition_variable m_cv;
    mutable std::atomic<int>        m_sig_count {0};
public:
    void WaitForReadySignal() const { std::unique_lock<std::mutex> lk(m_mtx); if(m_sig_count <= 0) m_cv.wait(lk); --m_sig_count; }
    void SignalReady()              { std::lock_guard<std::mutex> lk(m_mtx); ++m_sig_count; m_cv.notify_all(); }
    bool WaitForReadySignalTO(std::chrono::nanoseconds time_out) const // returns false if timeout elapses
        {
        std::unique_lock<std::mutex> lk(m_mtx);
        if(m_sig_count <= 0) { // Check that not signaled before the wait call
            if(m_cv.wait_for(lk, time_out) == std::cv_status::timeout)
                return false;
        }
        --m_sig_count;
        return true;
        }
};