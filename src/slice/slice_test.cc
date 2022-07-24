


#include <gtest/gtest.h>
#include "slice.h"


namespace latte {

    TEST(SliceTest, Empty) { Slice slice; }

    TEST(SliceTest, Simple) {
        latte::Slice s1 = "hello";
        std::string str("world111");
        latte::Slice s2 = str;
        ASSERT_EQ(s2.size() , 8);
    }

}  // namespace leveldb
