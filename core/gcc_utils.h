#pragma once

#include <features.h> // __GLIBC__

#if defined(_DEBUG) && defined(NDEBUG)
    #error "Both _DEBUG and NDEBUG are defined!"
#elif !defined(_DEBUG) && !defined(NDEBUG)
    #error "Neither _DEBUG nor NDEBUG are defined! "
#endif

#if defined(_DEBUG)
    #define IF_DEBUG(EXPR)  EXPR
    #define IF_RELEASE(EXPR)
    #define IF_DEBUG_ELSE(EXPR_DBG, EXPR_RLS) EXPR_DBG
#else
    #define IF_DEBUG(EXPR)
    #define IF_RELEASE(EXPR)  EXPR
    #define IF_DEBUG_ELSE(EXPR_DBG, EXPR_RLS) EXPR_RLS
#endif

// Returns the number of elements in a C array, e.g.
// double myArray[7];  COUNT_OF(myArray) == 7
#define COUNT_OF(C_ARRAY)           (sizeof(C_ARRAY)/sizeof((C_ARRAY)[0]))

// True if EXP is a power of 2: 2,4,8,16,32...
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

// Declare unique variables, for misc macros
#define MACRO_COMBINE_HELPER(X,Y)   X##Y
#define MACRO_COMBINE(X,Y)          MACRO_COMBINE_HELPER(X,Y)
#define UNIQUE_VAR(var)             MACRO_COMBINE(var,__LINE__)
