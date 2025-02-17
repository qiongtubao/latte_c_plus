


#include <gtest/gtest.h>

#include "string_util.h"

namespace latte {

    TEST(SliceTest, Empty) { Slice slice; }

    TEST(SliceTest, Simple) {
        latte::Slice s1 = "hello";
        std::string str("world111");
        latte::Slice s2 = str;
        ASSERT_EQ(s2.size() , 8);

        AppendNumberTo(&str, 1);
         ASSERT_EQ(str.size() , 9);

        std::unique_ptr<latte::Slice> slice_ptr(new Slice("hello"));

       


    }

}  // namespace leveldb
