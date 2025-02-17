



#ifndef __LATTE_C_PLUS_USER_WRITE_CALLBACK_H
#define __LATTE_C_PLUS_USER_WRITE_CALLBACK_H

#pragma once

namespace latte
{
    namespace rocksdb
    {
        // Custom callback functions to support users to plug in logic while data is
        // being written to the DB. It's intended for better synchronization between
        // concurrent writes. Note that these callbacks are in the write's critical path
        // It's desirable to keep them fast and minimum to not affect the write's
        // latency. These callbacks may be called in the context of a different thread.
        class UserWriteCallback {
            public:
            virtual ~UserWriteCallback() {}

            // This function will be called after the write is enqueued.
            virtual void OnWriteEnqueued() = 0;

            // This function will be called after wal write finishes if it applies.
            virtual void OnWalWriteFinish() = 0;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif