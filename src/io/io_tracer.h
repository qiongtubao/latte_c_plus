


#ifndef __LATTE_C_PLUS_IO_TRACER_H
#define __LATTE_C_PLUS_IO_TRACER_H

#include "port/lang.h"
#include "status/status.h"
#include "system/system_clock.h"
#include "monitoring/mutex/instrumented_mutex.h"
// #include "./io_trace_writer.h"

namespace latte
{

    // namespace rocksdb
    // {
        class IOTracer {
            public:
                IOTracer();
                ~IOTracer();
                // No copy and move.
                IOTracer(const IOTracer&) = delete;
                IOTracer& operator=(const IOTracer&) = delete;
                IOTracer(IOTracer&&) = delete;
                IOTracer& operator=(IOTracer&&) = delete;

                // no_sanitize is added for tracing_enabled. writer_ is protected under mutex
                // so even if user call Start/EndIOTrace and tracing_enabled is not updated in
                // the meanwhile, WriteIOOp will anyways check the writer_ protected under
                // mutex and ignore the operation if writer_is null. So its ok if
                // tracing_enabled shows non updated value.

                // Start writing IO operations to the trace_writer.
                // TSAN_SUPPRESSION Status
                // StartIOTrace(SystemClock* clock, const TraceOptions& trace_options,
                //             std::unique_ptr<TraceWriter>&& trace_writer);

                // Stop writing IO operations to the trace_writer.
                TSAN_SUPPRESSION void EndIOTrace();

                TSAN_SUPPRESSION bool is_tracing_enabled() const { return tracing_enabled; }

                // void WriteIOOp(const IOTraceRecord& record, IODebugContext* dbg);

                private:
                    // TraceOptions trace_options_;
                    // A mutex protects the writer_.
                    InstrumentedMutex trace_writer_mutex_;
                    // std::atomic<IOTraceWriter*> writer_;
                    // bool tracing_enabled is added to avoid costly operation of checking atomic
                    // variable 'writer_' is nullptr or not in is_tracing_enabled().
                    // is_tracing_enabled() is invoked multiple times by FileSystem classes.
                    bool tracing_enabled;
        };
    // } // namespace name
    
} // namespace latte

#endif