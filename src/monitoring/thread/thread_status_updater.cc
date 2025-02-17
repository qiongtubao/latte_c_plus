
#include "thread_status_updater.h"

namespace latte
{
    namespace rocksdb
    {
        
        void ThreadStatusUpdater::SetEnableTracking(bool enable_tracking) {
            auto* data = Get();
            if (data == nullptr) {
                return;
            }
            data->enable_tracking.store(enable_tracking, std::memory_order_relaxed);
        };

    } // namespace rocksdb
    
} // namespace latte
