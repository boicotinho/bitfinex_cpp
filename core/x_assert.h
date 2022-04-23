#pragma once
#include "gcc_utils.h"

#if 0 //def VALIDATE
//  TODO: Trap assertion failures. or maybe just route to <assert.h>
#else
#   define FAILURE(msg)              do{} while(0)
#   define ASSERT_MSG(x,msg)         do{} while(0)
#   define ASSERT(x)                 do{} while(0)
#   define ASSERT_EQ(x, y)           do{} while(0)
#   define ASSERT_NE(x, y)           do{} while(0)
#   define ASSERT_LT(x, y)           do{} while(0)
#   define ASSERT_LE(x, y)           do{} while(0)
#   define ASSERT_GT(x, y)           do{} while(0)
#   define ASSERT_GE(x, y)           do{} while(0)

#   define ASSERT_ALIGNED(ptr,algn)  do{} while(0)
#   define ASSERT_CACHE_ALIGNED(ptr) do{} while(0)
#   define ASSERT_IF(C,X)            do{} while(0)
#endif

#define STATIC_ASSERT(EXP) \
            static_assert((EXP), "Static assertion failed: " #EXP )

#define STATIC_ASSERT_CLASS_SIZE(SIZE)                      \
            void UNIQUE_VAR(assert__check_size)() const     \
            {                                               \
                static_assert(sizeof(*this)<=(SIZE),        \
                "Class larger than expected");              \
                static_assert(sizeof(*this)>=(SIZE),        \
                "Class smaller than expected");             \
            }

#define STATIC_ASSERT_ISPOW2(EXP) \
            static_assert(IS_POW2(EXP), "Expression is not a power of 2: " #EXP)
