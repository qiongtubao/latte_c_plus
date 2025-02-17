

#ifndef __LATTE_C_PLUS_CACHE_ALIGNED_INSTRUMENTED_MUTEX_H
#define __LATTE_C_PLUS_CACHE_ALIGNED_INSTRUMENTED_MUTEX_H

#pragma once 
#include "./instrumented_mutex.h"

namespace latte
{
    namespace rocksdb
    {
        // port::ALIGN_AS(CACHE_LINE_SIZE)
        class CacheAlignedInstrumentedMutex: 
            public InstrumentedMutex {
            
                
        };
    } // namespace rocksdb
    
} // namespace latte


#endif


