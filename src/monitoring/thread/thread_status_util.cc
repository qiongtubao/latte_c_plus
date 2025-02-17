


#include "thread_status_util.h"

#include "system/system_clock.h"

namespace latte
{
    namespace rocksdb
    {
        void ThreadStatusUtil::SetEnableTracking(bool enable_tracking) {
            if (thread_updater_local_cache_ == nullptr) {
                return;
            }
            thread_updater_local_cache_->SetEnableTracking(enable_tracking);
        }

        void ThreadStatusUtil::SetThreadOperation(ThreadStatus::OperationType op) {
            if (thread_updater_local_cache_ == nullptr) {
                return;
            }

            if (op != ThreadStatus::OP_UNKNOWN) {
                uint64_t current_time = SystemClock::Default()->NowMicros();
                thread_updater_local_cache_->SetOperationStartTime(current_time);
            } else {
                // TDOO(yhchiang): we could report the time when we set operation to
                // OP_UNKNOWN once the whole instrumentation has been done.
                thread_updater_local_cache_->SetOperationStartTime(0);
            }
            thread_updater_local_cache_->SetThreadOperation(op);
        }
    } // namespace rocksdb
    
} // namespace latte
