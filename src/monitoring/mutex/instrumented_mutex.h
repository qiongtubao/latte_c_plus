#ifndef __LATTE_C_PLUS_INSTRUMENTED_MUTEX_H
#define __LATTE_C_PLUS_INSTRUMENTED_MUTEX_H
#include "port/port_posix.h"

#pragma once 

namespace latte
{
    // namespace rocksdb
    // {
        class InstrumentedMutex {
            private:
                latte::port::Mutex mutex_;
            // Statistics* stats_;
            // SystemClock* clock_;
            public:
                void Lock();
                void AssertHeld() const { mutex_.AssertHeld(); }

        };
    // } // namespace rocksdb
    
} // namespace latte


#endif