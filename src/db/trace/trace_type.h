




#ifndef __LATTE_C_PLUS_TRACER_TYPE_H
#define __LATTE_C_PLUS_TRACER_TYPE_H

#include "status/status.h"
#include "./block_cache_trace_writer.h"
#include <memory>

namespace latte
{
    namespace rocksdb {

        // Supported trace record types.
        enum TraceType : char {
            kTraceNone = 0,
            kTraceBegin = 1,
            kTraceEnd = 2,
            // Query level tracing related trace types.
            kTraceWrite = 3,
            kTraceGet = 4,
            kTraceIteratorSeek = 5,
            kTraceIteratorSeekForPrev = 6,
            // Block cache tracing related trace types.
            kBlockTraceIndexBlock = 7,
            // TODO: split out kinds of filter blocks?
            kBlockTraceFilterBlock = 8,
            kBlockTraceDataBlock = 9,
            kBlockTraceUncompressionDictBlock = 10,
            kBlockTraceRangeDeletionBlock = 11,
            // IO tracing related trace type.
            kIOTracer = 12,
            // Query level tracing related trace type.
            kTraceMultiGet = 13,
            // All trace types should be added before kTraceMax
            kTraceMax,
        };
    };
    
} // namespace latte


#endif