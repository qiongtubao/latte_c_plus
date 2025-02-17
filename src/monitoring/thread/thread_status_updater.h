

#ifndef __LATTE_C_PLUS_THREAD_STATUS_UPDATER
#define __LATTE_C_PLUS_THREAD_STATUS_UPDATER


#include "thread_status_data.h"

namespace latte
{
    namespace rocksdb
    {
        class ThreadStatusUpdater {
            public:
                ThreadStatusUpdater() {}
                ~ThreadStatusUpdater() {}
                // 当前线程操作的开始时间。其格式为
                // 自某个固定时间点以来的微秒数。
                void SetOperationStartTime(const uint64_t start_time);
                // 更新当前线程的线程操作。
                void SetThreadOperation(const ThreadStatus::OperationType type);
                void SetEnableTracking(bool enable_tracking);
            protected:
                ThreadStatusData* Get() { return thread_status_data_; }

                static ThreadStatusData* thread_status_data_;
        };
    } // namespace rocksdb
    
} // namespace latte

#endif