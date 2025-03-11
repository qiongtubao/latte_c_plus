#include <gtest/gtest.h>
#include "status.h"


namespace latte {

    TEST(StatusTest, Empty) { 
        Status* status = new Status();
        
        delete status;
    }

}