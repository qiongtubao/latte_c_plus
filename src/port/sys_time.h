#include <time.h>
namespace latte
{
#if defined(OS_WIN) && (defined(_MSC_VER) || defined(__MINGW32__))

    struct TimeVal {
        long tv_sec;
        long tv_usec;
    };
    
#else
    
    using TimeVal = struct timeval;

#endif
    //获得时间的函数     
    void GetTimeOfDay(TimeVal* tv, struct  timezone* tz);
} // namespace latte