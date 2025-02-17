


#ifndef __LATTE_C_PLUS_TRIM_HISTORY_SCHEDULER_H
#define __LATTE_C_PLUS_TRIM_HISTORY_SCHEDULER_H

#include <atomic>
#include "column_family/column_family_mem_tables.h"
#include <mutex>
#include <set>
#include "../shared/autovector.h"
namespace latte
{
    namespace rocksdb
    {
        class TrimHistoryScheduler {
            public:
                TrimHistoryScheduler() : is_empty_(true) {}
                // When a column family needs history trimming, add cfd to the FIFO queue
                void ScheduleWork(ColumnFamilyData* cfd);

                // Remove the column family from the queue, the caller is responsible for
                // calling `MemtableList::TrimHistory`
                ColumnFamilyData* TakeNextColumnFamily();

                bool Empty();

                void Clear();

                // Not on critical path, use mutex to ensure thread safety
            private:
                std::atomic<bool> is_empty_;
                autovector<ColumnFamilyData*> cfds_;
                std::mutex checking_mutex_;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif