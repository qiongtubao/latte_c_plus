



#ifndef __LATTE_C_PLUS_REPORTER_H
#define __LATTE_C_PLUS_REPORTER_H

#include "status/status.h"

namespace latte
{
    struct Reporter {
        public:
            virtual ~Reporter();

            // Some corruption was detected.  "size" is the approximate number
            // of bytes dropped due to the corruption.
            virtual void Corruption(size_t bytes, const Status& status) = 0;

            virtual void OldLogRecord(size_t /*bytes*/) {}
    };
} // namespace latte


#endif