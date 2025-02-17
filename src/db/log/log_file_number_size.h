#ifndef __LATTE_C_PLUS_LOG_FILE_NUMBER_SIZE_H
#define __LATTE_C_PLUS_LOG_FILE_NUMBER_SIZE_H

#include <cinttypes>
namespace latte
{
    namespace rocksdb
    {
        struct LogFileNumberSize {
            explicit LogFileNumberSize(uint64_t _number) : number(_number) {}
            LogFileNumberSize() {}
            void AddSize(uint64_t new_size) { size += new_size; }
            uint64_t number;
            uint64_t size = 0;
            bool getting_flushed = false;
        };
    } // namespace rocksdb
    
} // namespace latte




#endif