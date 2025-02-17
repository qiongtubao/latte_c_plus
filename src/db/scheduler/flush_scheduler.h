


#ifndef __LATTE_C_PLUS_FLUSH_SCHEDULER_H
#define __LATTE_C_PLUS_FLUSH_SCHEDULER_H

#include <atomic>
#include "column_family/column_family_mem_tables.h"
#include <mutex>
#include <set>

namespace latte
{
    namespace rocksdb
    {
        class FlushScheduler {
            public:
                FlushScheduler() : head_(nullptr) {}
            private:
                struct Node {
                    ColumnFamilyData* column_family;
                    Node* next;
                };

                std::atomic<Node*> head_;
            #ifndef NDEBUG
                std::mutex checking_mutex_;
                std::set<ColumnFamilyData*> checking_set_;
            #endif  // NDEBUG
        };
    } // namespace rocksdb
    
} // namespace latte


#endif