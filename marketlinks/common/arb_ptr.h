#pragma once
#include "marketlinks/common/types.h"
#include "core/gcc_utils.h"
#include "core/cmpxchg16b.h"

// TODO: make it a generic class?
struct ArbPtr
{
    using PtrT    = BookTicker*;
    using TimeT   = UpdateId;
    using Compare = std::greater<TimeT>;

    bool atomic_update_if_newer(TimeT const a_uid, PtrT const a_ptr)
    {
        static_assert(sizeof(ArbRef) == 16,
            "Please check ArbRef for atomic 128 CAS")
        auto vv = m_ver;
        if(!Compare()(a_uid, vv))
            return false;
        auto pp = m_ptr;
        return cmpxchg16b(this, vv, pp, a_uid, a_ptr);
    }
private:
    TimeT volatile m_ver {}; // 18,541,854,003 : 35 bits?
    PtrT           m_ptr {};
} PACK128();
