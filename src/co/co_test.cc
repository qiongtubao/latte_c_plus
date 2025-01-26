#include "co.h"
#include <coroutine>
#include <stdexcept> // 添加此行以包含 runtime_error

#include <gtest/gtest.h>

namespace latte
{
    
    Task<int> myCoroutine(int value) {
        std::cout << "Doubling the number " << value << std::endl;
        co_return value * 2;
    }

    TEST(CoTest, Empty) {
        auto task = myCoroutine(5);
        int result = task.get(); // 获取协程的结果
        std::cout << "Result: " << result << std::endl; // 应输出 "Result: 10"
    }
} // namespace latte

