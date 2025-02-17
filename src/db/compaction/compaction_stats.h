
#ifndef __LATTE_C_PLUS_COMPACTION_STATS_H
#define __LATTE_C_PLUS_COMPACTION_STATS_H

#include <stdio.h>
namespace latte
{
    namespace leveldb
    {
        struct CompactionStats {
            CompactionStats() : micros(0), bytes_read(0), bytes_written(0) {}
            void Add(const CompactionStats& c) {
                this->micros += c.micros;
                this->bytes_read += c.bytes_read;
                this->bytes_written += c.bytes_written;
            }
            int64_t micros;
            int64_t bytes_read;
            int64_t bytes_written;
        };
    } // namespace leveldb
    
} // namespace latte

#endif