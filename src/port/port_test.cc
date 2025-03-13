#include <gtest/gtest.h>
#include "sys_time.h"

namespace latte
{
    TEST(SYS_TIME_TIME, Empty) {
        TimeVal now_tv;
        GetTimeOfDay(&now_tv, nullptr);
        assert(now_tv.tv_sec > 0);
    }
} // namespace latte
