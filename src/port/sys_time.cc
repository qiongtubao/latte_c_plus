#include "sys_time.h"

namespace latte
{

    #if defined(OS_WIN)

        void GetTimeOfDay(TimeVal* tv, struct timezone* /* tz */) {
            std::chrono::microseconds usNow(
                std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::system_clock::now().time_since_epoch()));

            std::chrono::seconds secNow(
                std::chrono::duration_cast<std::chrono::seconds>(usNow));

            tv->tv_sec = static_cast<long>(secNow.count());
            tv->tv_usec = static_cast<long>(
            usNow.count() -
            std::chrono::duration_cast<std::chrono::microseconds>(secNow).count());
        }
    #else 
        #include <sys/time.h>
        void GetTimeOfDay(TimeVal* tv, struct timezone* tz) {
            gettimeofday(tv, tz);
        }
    #endif
} // namespace latte
