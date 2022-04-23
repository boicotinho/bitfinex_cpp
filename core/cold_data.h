#pragma once
#include "thread_utils.h"
#include "x_assert.h"
#include "scope_exit.h"
#include "gcc_utils.h"
#include <unordered_map>
#include <memory>

// Helps move cold data away from hot data, improving cache locality.
template <class TOwner, class TCold, class MutexType = SleepMutex>
class ColdDataRef
{
public:
    template <class... TArgs>
    static TCold& init(TOwner const* owner, TArgs&&... args)
        {
        GlobalMap& g_map = instance();
        std::unique_lock<MutexType> lk(g_map.mtx);
        std::unique_ptr<TCold> uptr (new TCold(std::forward<TArgs>(args)...));
        TCold* const pp = uptr.get();
        g_map.cold_map[owner] = std::move(uptr);
        return *pp;
        }

    static TCold* get(TOwner const* owner) __attribute__ ((pure))
        {
        GlobalMap& g_map = instance();
        const auto it = g_map.cold_map.find(owner);
        if(UNLIKELY(it == g_map.cold_map.end()))
            return nullptr;
        return it->second.get();
        }

    static void release(TOwner const* owner) noexcept
        { SWALLOW_EXCEPTIONS(release_inner(owner)); }

private:
    static void release_inner(TOwner const* owner)
        {
        GlobalMap& g_map = instance();
        std::unique_lock<MutexType> lk(g_map.mtx);
        g_map.cold_map.erase(owner);
        }
private:
    struct GlobalMap
    {
        MutexType mtx;
        std::unordered_map<TOwner const*, std::unique_ptr<TCold>> cold_map;
    };
    static GlobalMap& instance() {static GlobalMap g_inst; return g_inst;}
};
