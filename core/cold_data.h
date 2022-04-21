#pragma once
#include "thread_utils.h"
#include "x_assert.h"
#include "scope_exit.h"
#include "gcc_utils.h"
#include <unordered_map>
#include <memory>

// Helps move cold data away from hot data, improving cache locality.
template <class TOwner, class TCold>
class ColdDataRef
{
public:
    template <class... TArgs>
    static TCold& InitColdData(TOwner const* owner, TArgs&&... args)
        {
        GlobalMap& g_map = Instance();
        std::unique_lock<SpinMutex> lk(g_map.mtx);
        std::unique_ptr<TCold> uptr (new TCold(std::forward<TArgs>(args)...));
        TCold* const pp = uptr.get();
        g_map.cold_map[owner] = std::move(uptr);
        return *pp;
        }

    static TCold* Get(TOwner const* owner)
        {
        GlobalMap& g_map = Instance();
        const auto it = g_map.cold_map.find(owner);
        if(UNLIKELY(it == g_map.cold_map.end()))
            return nullptr;
        return it->second.get();
        }

    static void ReleaseColdData(TOwner const* owner) noexcept
        { SWALLOW_EXCEPTIONS(ReleaseColdDataInner(owner)); }

private:
    static void ReleaseColdDataInner(TOwner const* owner)
        {
        GlobalMap& g_map = Instance();
        std::unique_lock<SpinMutex> lk(g_map.mtx);
        g_map.cold_map.erase(owner);
        }
private:
    struct GlobalMap
    {
        SpinMutex mtx;
        std::unordered_map<TOwner const*, std::unique_ptr<TCold>> cold_map;
    };
    static GlobalMap& Instance() {static GlobalMap g_inst; return g_inst;}
};
