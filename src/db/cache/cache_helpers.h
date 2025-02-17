


#ifndef __LATTE_C_PLUS_CACHE_HELPERS_H
#define __LATTE_C_PLUS_CACHE_HELPERS_H

#include "cache/cache.h"
#include <cassert>
namespace latte
{
    namespace rocksdb
    {
        void ReleaseCacheHandleCleanup(void* arg1, void* arg2);
    } // namespace rocksdb
    
} // namespace latte


#endif