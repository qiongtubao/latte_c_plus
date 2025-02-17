
#ifndef __LATTE_C_PLUS_LOG_REPORTER_H
#define __LATTE_C_PLUS_LOG_REPORTER_H

#include "../reporter/reporter.h"
namespace latte
{
    

    struct LogReporter : public Reporter {
        Status* status;
        void Corruption(size_t /*bytes*/, const Status& s) override {
            if (status->ok()) {
                *status = s;
            }
        }
    };
} // namespace latte


                



#endif