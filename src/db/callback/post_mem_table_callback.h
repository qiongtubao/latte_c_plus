


#ifndef __LATTE_C_PLUS_POST_MEM_TABLE_CALLBACK_H
#define __LATTE_C_PLUS_POST_MEM_TABLE_CALLBACK_H

#pragma once
#include "status/status.h"
#include "../types.h"

namespace latte
{
    namespace rocksdb
    {
        // Callback invoked after finishing writing to the memtable but before
        // publishing the sequence number to readers.
        // Note that with write-prepared/write-unprepared transactions with
        // two-write-queues, PreReleaseCallback is called before publishing the
        // sequence numbers to readers.
        class PostMemTableCallback {
            public:
            virtual ~PostMemTableCallback() {}

            virtual Status operator()(SequenceNumber seq, bool disable_memtable) = 0;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif