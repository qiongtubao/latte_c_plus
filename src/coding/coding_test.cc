

#include <gtest/gtest.h>
#include "coding.h"

namespace latte
{
    
    TEST(CodingTest, Empty) {
        std::string buffer = "";
        PutVarint32(&buffer, 10);
        Slice slice = buffer;
        uint32_t v;
        GetVarint32(&slice, &v);
        ASSERT_EQ(v , 10);
    }
}