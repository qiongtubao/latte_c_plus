
#include <coroutine>
#include <stdexcept> // 添加此行以包含 runtime_error

#include <gtest/gtest.h>
#include "latte.h"
namespace latte
{
    namespace A
    {
        TEST(ATest, Empty) {
            L* result = new L("a");
            result->hello();
            delete result;
        }
    } // namespace A

    namespace B
    {
        TEST(ATest, Empty) {
            L* result = new L("b");
            result->hello();
            delete result;
        }
    } // namespace B
    
    
} //namespace latte