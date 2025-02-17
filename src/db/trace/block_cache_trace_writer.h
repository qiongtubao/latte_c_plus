

#ifndef __LATTE_C_PLUS_BLOCK_CACHE_TRACER_OPTIONS_H
#define __LATTE_C_PLUS_BLOCK_CACHE_TRACER_OPTIONS_H

#include "status/status.h"
#include "slice/slice.h"
#include "block_cache_trace_record.h"

namespace latte
{
    namespace rocksdb
    {
        // BlockCacheTraceWriter is an abstract class that captures all RocksDB block
        // cache accesses. Every RocksDB operation is passed to WriteBlockAccess()
        // with a BlockCacheTraceRecord.
        class BlockCacheTraceWriter {
            public:
                virtual ~BlockCacheTraceWriter() {}

                // Pass Slice references to avoid copy.
                virtual Status WriteBlockAccess(const BlockCacheTraceRecord& record,
                                                const Slice& block_key, const Slice& cf_name,
                                                const Slice& referenced_key) = 0;

                // Write a trace header at the beginning, typically on initiating a trace,
                // with some metadata like a magic number and RocksDB version.
                virtual Status WriteHeader() = 0;
        };
    } // namespace rocksdb
       
} // namespace latte


#endif