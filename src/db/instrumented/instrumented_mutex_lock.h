


#ifndef __LATTE_C_PLUS_INSTRUMENTED_MUTEX_LOCK_H
#define __LATTE_C_PLUS_INSTRUMENTED_MUTEX_LOCK_H

#include "port/port_example.h"
#include "system/system_clock.h"
#include "statistics/statistics.h"
namespace latte
{
    namespace rocksdb
    {
        class InstrumentedMutex {
            public:
                explicit InstrumentedMutex(bool adaptive = false)
                    : mutex_(adaptive), stats_(nullptr), clock_(nullptr), stats_code_(0) {}

                explicit InstrumentedMutex(SystemClock* clock, bool adaptive = false)
                    : mutex_(adaptive), stats_(nullptr), clock_(clock), stats_code_(0) {}

                InstrumentedMutex(Statistics* stats, SystemClock* clock, int stats_code,
                                    bool adaptive = false)
                    : mutex_(adaptive),
                        stats_(stats),
                        clock_(clock),
                        stats_code_(stats_code) {}

                #ifdef COERCE_CONTEXT_SWITCH
                InstrumentedMutex(Statistics* stats, SystemClock* clock, int stats_code,
                                    InstrumentedCondVar* bg_cv, bool adaptive = false)
                    : mutex_(adaptive),
                        stats_(stats),
                        clock_(clock),
                        stats_code_(stats_code),
                        bg_cv_(bg_cv) {}
                #endif

                void Lock();

                void Unlock() { mutex_.Unlock(); }

                void AssertHeld() const { mutex_.AssertHeld(); }

            private:
                void LockInternal();
                friend class InstrumentedCondVar;
                port::Mutex mutex_;
                Statistics* stats_;
                SystemClock* clock_;
                int stats_code_;
                #ifdef COERCE_CONTEXT_SWITCH
                InstrumentedCondVar* bg_cv_ = nullptr;
                #endif
        };
        class InstrumentedMutexLock {
            public:
                explicit InstrumentedMutexLock(InstrumentedMutex* mutex) : mutex_(mutex) {
                    mutex_->Lock();
                }
                
                ~InstrumentedMutexLock() { mutex_->Unlock(); }
            private:
                InstrumentedMutex* const mutex_;
                InstrumentedMutexLock(const InstrumentedMutexLock&) = delete;
                void operator=(const InstrumentedMutexLock&) = delete;
        };
    } // namespace rocksdb
    
} // namespace latte





#endif