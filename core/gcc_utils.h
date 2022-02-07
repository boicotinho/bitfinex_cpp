#pragma once

#define IS_POW2(EXP)                (((EXP)&((EXP)-1))==0)
#define PACK64()                    __attribute__((aligned(64)))
#define COMPILER_BARRIER()          __asm__ __volatile("")
#define UNLIKELY(cond)              __builtin_expect((bool)(cond), 0)
#define LIKELY(cond)                __builtin_expect((bool)(cond), 1)
#define ASSUME(cond)                do { if(!(cond)) __builtin_unreachable(); } while(0)

#ifdef NDEBUG
    #define FORCE_INLINE            __attribute__((always_inline)) inline
#else
    #define FORCE_INLINE            inline // Breakpoints suck if forced inline in debug
#endif
