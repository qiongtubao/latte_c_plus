


#ifndef __LATTE_C_PLUS_WRITE_CALLBACK_H
#define __LATTE_C_PLUS_WRITE_CALLBACK_H

#include "status/status.h"
#include "../db.h"
namespace latte
{
    namespace rocksdb
    {
        class WriteCallback {
            public:
            virtual ~WriteCallback() {}

            // Will be called while on the write thread before the write executes.  If
            // this function returns a non-OK status, the write will be aborted and this
            // status will be returned to the caller of DB::Write().
            virtual Status Callback(DB* db) = 0;

            // return true if writes with this callback can be batched with other writes
            virtual bool AllowWriteBatching() = 0;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif