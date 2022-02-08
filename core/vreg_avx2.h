#pragma once
#include "core/x_assert.h"
#include "core/tpl_intrinsics.h"
#include <limits>
#include <type_traits>
#include <x86intrin.h>

namespace cpu_avx2
{
template<typename T_SCALAR>
union VReg
{
    using scalar_t = T_SCALAR;
    using vector_t = __m256i;
    enum : size_t { LEN = sizeof(vector_t) / sizeof(scalar_t) };

    vector_t vec;
    scalar_t scalars[LEN];

    static VReg LoadUnaligned(const scalar_t* src)
        { return _mm256_loadu_si256((const __m256i*) src); }

    static VReg LoadAligned(const scalar_t* src)
        {
        ASSERT_ALIGNED(src, sizeof(vector_t));
        return _mm256_load_si256((const __m256i*) src);
        }

public: // Various constructors / assignment operators

    constexpr VReg(const VReg& rhs) : vec(rhs.vec) {}
    constexpr VReg(vector_t rhs) : vec(rhs) {}
    VReg& operator=(const VReg& rhs) { vec = rhs.vec; return *this; }
    VReg& operator=(vector_t rhs)    { vec = rhs; return *this; }
    operator vector_t() const {return vec;}

    static VReg MakeBroadcast(scalar_t val) {return tpl_intrinsics::Broadcast<scalar_t, vector_t>::op(val);}
    VReg CmpEq(const VReg& rhs) const {return tpl_intrinsics::CmpEq<scalar_t, vector_t>::op(vec, rhs.vec);}
    uint32_t MoveMask() const {return tpl_intrinsics::MoveMask<vector_t>::op(vec);}

    struct u8{};  // tag for  8-bit scalars (32x) on 256-bit vector
    struct u16{}; // tag for 16 bit scalars (16x) on 256-bit vector
    struct u32{}; // tag for 32 bit scalars  (8x) on 256-bit vector
    struct u64{}; // tag for 64 bit scalars  (4x) on 256-bit vector
    //using uu = typename std::conditional<(int)LEN==32, u8, u16>::type;
    using uu = typename std::conditional<(int)LEN==32, u8, 
               typename std::conditional<(int)LEN==16, u16,
               typename std::conditional<(int)LEN== 8, u32,
               u64
               >::type
               >::type
               >::type;

    constexpr explicit VReg(scalar_t val = 0) : VReg(val, uu()) {}
private:
    constexpr explicit VReg(scalar_t val, u8) // typename std::enable_if<(int)LEN==32>::type* = nullptr
        : scalars{ val,val,val,val, val,val,val,val
                 , val,val,val,val, val,val,val,val
                 , val,val,val,val, val,val,val,val
                 , val,val,val,val, val,val,val,val
                 } {}

    constexpr explicit VReg(scalar_t val, u16)
        : scalars{ val,val,val,val, val,val,val,val
                 , val,val,val,val, val,val,val,val
                 } {}

    constexpr explicit VReg(scalar_t val, u32)
        : scalars{ val,val,val,val, val,val,val,val
                 } {}

    constexpr explicit VReg(scalar_t val, u64)
        : scalars{ val,val,val,val,
                 } {}
};

} // namespace cpu_avx2
