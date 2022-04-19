#pragma once

#define IS_POW2(EXP)                (((EXP)&((EXP)-1))==0)
#define PACK64()                    __attribute__((aligned(64)))
#define PACK128()                   __attribute__((aligned(128)))
#define COMPILER_BARRIER()          __asm__ __volatile("")

// Branch optimization macros
#define UNLIKELY(cond)              __builtin_expect((bool)(cond), 0)
#define LIKELY(cond)                __builtin_expect((bool)(cond), 1)
#define ASSUME(cond)                do { if(!(cond)) __builtin_unreachable(); } while(0)

// Alignment macros
#define UNIT_COUNT_UP(val,unit_size)(((val)+((unit_size)-1))/(unit_size))
#define ALIGN_UP(val,alignment)     (UNIT_COUNT_UP(val,alignment) * (alignment))
#define ALIGN_DOWN(val,alignment)   ((val)/(alignment)*(alignment))
#define IS_ALIGNED(val,alignment)   ((uintptr_t(val) & ((alignment)-1)) == 0)

#ifdef NDEBUG
    #define FORCE_INLINE            __attribute__((always_inline)) inline
#else
    #define FORCE_INLINE            inline // Breakpoints suck if forced inline in debug
#endif
