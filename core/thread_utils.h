#pragma once
#include <atomic>
#include <mutex>
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
