
#ifndef __LATTE_C_PLUS_PORT_EXAMPLE_H
#define __LATTE_C_PLUS_PORT_EXAMPLE_H
#include "./thread_annotations.h"


namespace latte
{
    
    namespace port
    {   
        class LOCKABLE Mutex {
            public:
                Mutex();
                ~Mutex();
            void Lock() EXCLUSIVE_LOCK_FUNCTION();

            // Unlock the mutex.
            // REQUIRES: This mutex was locked by this thread.
            void Unlock() UNLOCK_FUNCTION();

            // Optionally crash if this thread does not hold this mutex.
            // The implementation must be fast, especially if NDEBUG is
            // defined.  The implementation is allowed to skip all checks.
            void AssertHeld() ASSERT_EXCLUSIVE_LOCK();
        };
    } // namespace port

} // namespace latte

#endif