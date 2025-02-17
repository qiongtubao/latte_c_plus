




#ifndef __LATTE_C_PLUS_BLOCK_CACHE_TRACER_H
#define __LATTE_C_PLUS_BLOCK_CACHE_TRACER_H

#include "status/status.h"
#include "./block_cache_trace_writer.h"
#include "./block_cache_trace_record.h"
#include "../instrumented/instrumented_mutex_lock.h"
#include <memory>

namespace latte
{
    namespace rocksdb {

        // Options for tracing block cache accesses
        struct BlockCacheTraceOptions {
            // Specify trace sampling option, i.e. capture one per how many requests.
            // Default to 1 (capture every request).
            uint64_t sampling_frequency = 1;
        };
        // A block cache tracer. It downsamples the accesses according to
        // trace_options and uses BlockCacheTraceWriter to write the access record to
        // the trace file.
        class BlockCacheTracer {
            public:
                BlockCacheTracer();
                ~BlockCacheTracer();
                // No copy and move.
                BlockCacheTracer(const BlockCacheTracer&) = delete;
                BlockCacheTracer& operator=(const BlockCacheTracer&) = delete;
                BlockCacheTracer(BlockCacheTracer&&) = delete;
                BlockCacheTracer& operator=(BlockCacheTracer&&) = delete;

                // Start writing block cache accesses to the trace_writer.
                Status StartTrace(const BlockCacheTraceOptions& trace_options,
                                    std::unique_ptr<BlockCacheTraceWriter>&& trace_writer);

                // Stop writing block cache accesses to the trace_writer.
                void EndTrace();

                bool is_tracing_enabled() const {
                    return writer_.load(std::memory_order_relaxed);
                }

                Status WriteBlockAccess(const BlockCacheTraceRecord& record,
                                        const Slice& block_key, const Slice& cf_name,
                                        const Slice& referenced_key);

                // GetId cycles from 1 to std::numeric_limits<uint64_t>::max().
                uint64_t NextGetId();

                private:
                BlockCacheTraceOptions trace_options_;
                // A mutex protects the writer_.
                InstrumentedMutex trace_writer_mutex_;
                std::atomic<BlockCacheTraceWriter*> writer_;
                std::atomic<uint64_t> get_id_counter_;
        };
    };
    
} // namespace latte


#endif