#pragma once
//#include "marketlinks/common/types.h"
#include "core/gcc_utils.h"
#include "core/cmpxchg16b.h"
#include <functional>

// TODO: make it a generic class?
template< class ValueT
        , class TimeT
        , class Compare = std::greater<TimeT>
        >
struct ArbPtr
{
    //using ValueT  = BookTicker;
    //using TimeT   = UpdateId;
    //using Compare = std::greater<TimeT>;

    constexpr ArbPtr() = default;

    constexpr ArbPtr(TimeT a_version, ValueT* a_ptr)
        : m_ver(a_version), m_ptr(a_ptr) {}

    bool atomic_update_if_newer(TimeT const a_version, ValueT* const a_ptr)
    {
        static_assert(sizeof(ArbPtr) == 16,
            "Please check ArbPtr for atomic 128 CAS")
        auto vv = m_ver;
        if(!Compare()(a_version, vv))
            return false;
        auto pp = m_ptr;
        return cmpxchg16b(this, vv, pp, a_version, a_ptr);
    }

    ValueT* get() {return m_ptr;}
    ValueT const* get() const {return m_ptr;}

    ValueT* operator->() {return m_ptr;}
    ValueT const* operator->() const {return m_ptr;}

    ValueT& operator*() {return *m_ptr;}
    ValueT const& operator->() const {return *m_ptr;}

    constexpr explicit operator bool() const {return !!m_ptr;}

private:
    TimeT volatile m_ver {}; // 18,541,854,003 : 35 bits?
    ValueT*        m_ptr {};
} PACK128();
