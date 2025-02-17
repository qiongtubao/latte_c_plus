



#ifndef __LATTE_C_PLUS_WRITE_STALL_INFO_H
#define __LATTE_C_PLUS_WRITE_STALL_INFO_H

#include <string>
#include "../types.h"

namespace latte
{
    namespace rocksdb
    {
        struct WriteStallInfo {
            // the name of the column family
            std::string cf_name;
            // state of the write controller
            struct {
                WriteStallCondition cur;
                WriteStallCondition prev;
            } condition;
        };
    } // namespace rocksdb
    
} // namespace latte



#endif