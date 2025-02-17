#include <gtest/gtest.h>
#include "status.h"


namespace latte {

    TEST(StatusTest, EQ) { 
        Status s0;
        s0.code_ = Status::Code::kNotFound;
        s0.subcode_ = Status::SubCode::kMutexTimeout;
        
        Status* status = new Status();
        // new Status(Status::Code::kNotFound, Status::SubCode::kMutexTimeout);
        status = &s0;
        ASSERT_EQ(status->code(), Status::Code::kNotFound);
        // Status* s0 = status;
        // ASSERT_EQ(status->code(), Status::Code::kOk);
        // // ASSERT_EQ(status->subcode(), Status::SubCode::kNone);
        // ASSERT_EQ(s0->code(), Status::Code::kNotFound);
        // ASSERT_EQ(s0->code(), Status::Code::kNotFound);
        // ASSERT_EQ(s0->subcode(), Status::SubCode::kMutexTimeout);
        delete status;
    }



}