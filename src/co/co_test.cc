#include "co.h"
#include <coroutine>
#include <stdexcept> // 添加此行以包含 runtime_error

#include <gtest/gtest.h>

namespace latte
{
    
    Task<long> myCoroutine(long value) {
        long result = value * 2;
        co_return result;
    }

    TEST(CoTest, Empty) {
        // auto task = ;
        Task<long> task =  myCoroutine(5);
        long result = task.get();
        std::cout <<"!!!"<<std::endl;
        EXPECT_EQ(result, 10);
        std::cout <<"???"<<std::endl;
    }
    Task<void> TestCoroutLine() {
        long result = co_await myCoroutine(10);
        EXPECT_EQ(result, 20);
    }
} // namespace latte 

