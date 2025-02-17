



#ifndef __LATTE_C_PLUS_THREAD_STATUS_DATA
#define __LATTE_C_PLUS_THREAD_STATUS_DATA

#include <atomic>
#include "thread_status.h"

namespace latte
{
    namespace rocksdb
    {
        struct ThreadStatusData {
            explicit ThreadStatusData() {
                enable_tracking.store(false);
                thread_id.store(0);
                thread_type.store(ThreadStatus::USER);
                cf_key.store(nullptr);
                operation_type.store(ThreadStatus::OP_UNKNOWN);
                op_start_time.store(0);
                state_type.store(ThreadStatus::STATE_UNKNOWN);
            }

            // A flag to indicate whether the thread tracking is enabled
            // in the current thread.
            // If set to false, then SetThreadOperation and SetThreadState
            // will be no-op.
            std::atomic<bool> enable_tracking;

            std::atomic<uint64_t> thread_id;
            std::atomic<ThreadStatus::ThreadType> thread_type;
            std::atomic<void*> cf_key;
            std::atomic<ThreadStatus::OperationType> operation_type;
            std::atomic<uint64_t> op_start_time;
            std::atomic<ThreadStatus::OperationStage> operation_stage;
            std::atomic<uint64_t> op_properties[ThreadStatus::kNumOperationProperties];
            std::atomic<ThreadStatus::StateType> state_type;
        };

    } // namespace rocksdb
    
} // namespace latte


#endif