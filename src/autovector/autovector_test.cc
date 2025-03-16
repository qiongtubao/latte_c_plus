


#include <gtest/gtest.h>
#include "autovector.h"

namespace latte
{
    
    TEST(AutovectorTest, Empty) {
        autovector<int>* result = new autovector<int>();
        result->push_back(0);
        result->push_back(1);
        int i = 0;
        auto it = result->begin();
        while(it != result->end()) {
            ASSERT_EQ(*it, i);
            i++;
            it++;
        }
    }
}