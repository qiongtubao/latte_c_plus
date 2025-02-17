

#ifndef __LATTE_C_PLUS_INTERNAL_STATS_H
#define __LATTE_C_PLUS_INTERNAL_STATS_H



#include <map>
#include <string>

namespace latte
{
    namespace rocksdb
    {

        enum class LevelStatType {
            INVALID = 0,
            NUM_FILES,
            COMPACTED_FILES,
            SIZE_BYTES,
            SCORE,
            READ_GB,
            RN_GB,
            RNP1_GB,
            WRITE_GB,
            W_NEW_GB,
            MOVED_GB,
            WRITE_AMP,
            READ_MBPS,
            WRITE_MBPS,
            COMP_SEC,
            COMP_CPU_SEC,
            COMP_COUNT,
            AVG_SEC,
            KEY_IN,
            KEY_DROP,
            R_BLOB_GB,
            W_BLOB_GB,
            TOTAL  // total number of types
        };

        struct LevelStat {
            // This what will be L?.property_name in the flat map returned to the user
            std::string property_name;
            // This will be what we will print in the header in the cli
            std::string header_name;
        };
        class InternalStats {
            public:
                static const std::map<LevelStatType, LevelStat> compaction_level_stats;

                enum InternalCFStatsType {
                    MEMTABLE_LIMIT_DELAYS,
                    MEMTABLE_LIMIT_STOPS,
                    L0_FILE_COUNT_LIMIT_DELAYS,
                    L0_FILE_COUNT_LIMIT_STOPS,
                    PENDING_COMPACTION_BYTES_LIMIT_DELAYS,
                    PENDING_COMPACTION_BYTES_LIMIT_STOPS,
                    // Write slowdown caused by l0 file count limit while there is ongoing L0
                    // compaction
                    L0_FILE_COUNT_LIMIT_DELAYS_WITH_ONGOING_COMPACTION,
                    // Write stop caused by l0 file count limit while there is ongoing L0
                    // compaction
                    L0_FILE_COUNT_LIMIT_STOPS_WITH_ONGOING_COMPACTION,
                    WRITE_STALLS_ENUM_MAX,
                    // End of all write stall stats
                    BYTES_FLUSHED,
                    BYTES_INGESTED_ADD_FILE,
                    INGESTED_NUM_FILES_TOTAL,
                    INGESTED_LEVEL0_NUM_FILES_TOTAL,
                    INGESTED_NUM_KEYS_TOTAL,
                    INTERNAL_CF_STATS_ENUM_MAX,
                };

                enum InternalDBStatsType {
                    kIntStatsWalFileBytes,
                    kIntStatsWalFileSynced,
                    kIntStatsBytesWritten,
                    kIntStatsNumKeysWritten,
                    kIntStatsWriteDoneByOther,
                    kIntStatsWriteDoneBySelf,
                    kIntStatsWriteWithWal,
                    // TODO(hx235): Currently `kIntStatsWriteStallMicros` only measures
                    // "delayed" time of CF-scope write stalls, not including the "stopped" time
                    // nor any DB-scope write stalls (e.g, ones triggered by
                    // `WriteBufferManager`).
                    //
                    // However, the word "write stall" includes both "delayed" and "stopped"
                    // (see `WriteStallCondition`) and DB-scope writes stalls (see
                    // `WriteStallCause`).
                    //
                    // So we should improve, rename or clarify it
                    kIntStatsWriteStallMicros,
                    kIntStatsWriteBufferManagerLimitStopsCounts,
                    kIntStatsNumMax,
                };


        };
    } // namespace rocksdb
    
} // namespace latte


#endif