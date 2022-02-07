#pragma once

#ifdef VALIDATE
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
