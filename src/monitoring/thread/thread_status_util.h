
#ifndef __LATTE_C_PLUS_THREAD_STATUS_UTIL
#define __LATTE_C_PLUS_THREAD_STATUS_UTIL

#include "thread_status.h"
#include "thread_status_updater.h"
#include <iostream>
#include <thread>
#include <string>
#include "thread_local.h"

namespace latte
{
    
    namespace rocksdb
    {
        struct Entry {
            Entry(): ptr(nullptr) {}
            Entry(const Entry& e) : ptr(e.ptr.load(std::memory_order_relaxed)) {}
            std::atomic<void*> ptr;
        };

        struct ThreadData {
            explicit ThreadData(StaticMeta* _inst):
                entries(), next(nullptr), prev(nullptr), inst(_inst) {}
            std::vector<Entry> entries;
            ThreadData* next;
            ThreadData* prev;
            StaticMeta* inst;
        };
        class ThreadStatusUtil {
            public:
                static void SetEnableTracking(bool enable_tracking);
                static void SetThreadOperation(ThreadStatus::OperationType type);
                static void ResetThreadStatus();
            

                static thread_local ThreadStatusUpdater* thread_updater_local_cache_;

        };
    } // namespace rocksdb
    
} // namespace latte

#endif
