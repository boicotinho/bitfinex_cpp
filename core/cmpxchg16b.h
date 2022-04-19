#pragma once
#include <stdint.h>
#include "gcc_utils.h"

// https://www.felixcloutier.com/x86/cmpxchg8b:cmpxchg16b
// https://developers.redhat.com/blog/2016/02/25/new-asm-flags-feature-for-x86-in-gcc-6
FORCE_INLINE
bool cmpxchg16b( void*      const addr     // 2x 64-bit vals, aligned to 128-bit
               , uint64_t&        old_val1 // if failed, update to match addr
               , uint64_t&        old_val2 // if failed, update to match addr
               , uint64_t   const new_val1
               , uint64_t   const new_val2
               )
{
    char result;
    // Compare RDX:RAX with m128.
    // If equal, set ZF and load RCX:RBX into m128.
    // Else, clear ZF and load m128 into RDX:RAX.
#if defined(__GCC_ASM_FLAG_OUTPUTS__)
    __asm__ __volatile__(
        "lock; "
        "cmpxchg16b %0"
        : "+m" (*(uint64_t*)addr), "=@ccz" (result),
          "+d" (old_val2),         "+a"    (old_val1)
        : "c"  (new_val2),         "b"     (new_val1)
        //: "memory"
        );
#else
    bool dummy; // an output for clobbered rdx
    __asm__ __volatile__(
        "lock; "
        "cmpxchg16b %0; "
        "setz %1"
        : "+m" (*(uint64_t*)addr), "=a" (result), "=d" (dummy)
        : "d"  (old_val2),         "a"  (old_val1),
          "c"  (new_val2),         "b"  (new_val1)
        //: "memory"
        );
#endif
    return  result;
}
