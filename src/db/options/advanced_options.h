


#ifndef __LATTE_C_PLUS_ADVANCED_OPTIONS_H
#define __LATTE_C_PLUS_ADVANCED_OPTIONS_H


namespace latte
{
    namespace rocksdb
    {
        enum CompactionStyle : char {
            // level based compaction style
            kCompactionStyleLevel = 0x0,
            // Universal compaction style
            kCompactionStyleUniversal = 0x1,
            // FIFO compaction style
            kCompactionStyleFIFO = 0x2,
            // Disable background compaction. Compaction jobs are submitted
            // via CompactFiles().
            kCompactionStyleNone = 0x3,
        };

        enum class PrepopulateBlobCache : uint8_t {
            kDisable = 0x0,    // Disable prepopulate blob cache
            kFlushOnly = 0x1,  // Prepopulate blobs during flush only
        };

    } // namespace rocksdb
    
} // namespace latte



#endif