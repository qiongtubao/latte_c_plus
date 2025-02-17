




#ifndef __LATTE_C_PLUS_SECONDARY_CACHE_RESULT_HANDLE_H
#define __LATTE_C_PLUS_SECONDARY_CACHE_RESULT_HANDLE_H
#include "cache.h"

namespace latte
{
    namespace rocksdb
    {
        class SecondaryCacheResultHandle {
            public:
                virtual ~SecondaryCacheResultHandle() = default;

                // Returns whether the handle is ready or not
                virtual bool IsReady() = 0;

                // Block until handle becomes ready
                virtual void Wait() = 0;

                // Return the cache entry object (also known as value). If nullptr, it means
                // the lookup was unsuccessful.
                virtual Cache::ObjectPtr Value() = 0;

                // Return the out_charge from the helper->create_cb used to construct the
                // object.
                // WART: potentially confusing name
                virtual size_t Size() = 0;
        };
    } // namespace rocksdb
    
} // namespace latte

#endif