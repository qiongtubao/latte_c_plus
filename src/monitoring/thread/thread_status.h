
#ifndef __LATTE_C_PLUS_THREAD_STATUS
#define __LATTE_C_PLUS_THREAD_STATUS


namespace latte
{
    namespace rocksdb
    {
        // TODO(yhchiang): remove this function once c++14 is available
        //                 as std::max will be able to cover this.
        // Current MS compiler does not support constexpr
        template <int A, int B>
        struct constexpr_max {
            static const int result = (A > B) ? A : B;
        };
        struct ThreadStatus {
            public:
                enum ThreadType : int {
                    HIGH_PRIORITY = 0,  // RocksDB BG thread in high-pri thread pool
                    LOW_PRIORITY,       // RocksDB BG thread in low-pri thread pool
                    USER,               // User thread (Non-RocksDB BG thread)
                    BOTTOM_PRIORITY,    // RocksDB BG thread in bottom-pri thread pool
                    NUM_THREAD_TYPES
                };
                enum OperationType : int {
                    OP_UNKNOWN = 0,
                    OP_COMPACTION,
                    OP_FLUSH,
                    OP_DBOPEN,
                    OP_GET,
                    OP_MULTIGET,
                    OP_DBITERATOR,
                    OP_VERIFY_DB_CHECKSUM,
                    OP_VERIFY_FILE_CHECKSUMS,
                    OP_GETENTITY,
                    OP_MULTIGETENTITY,
                    NUM_OP_TYPES
                };

                enum OperationStage : int {
                    STAGE_UNKNOWN = 0,
                    STAGE_FLUSH_RUN,
                    STAGE_FLUSH_WRITE_L0,
                    STAGE_COMPACTION_PREPARE,
                    STAGE_COMPACTION_RUN,
                    STAGE_COMPACTION_PROCESS_KV,
                    STAGE_COMPACTION_INSTALL,
                    STAGE_COMPACTION_SYNC_FILE,
                    STAGE_PICK_MEMTABLES_TO_FLUSH,
                    STAGE_MEMTABLE_ROLLBACK,
                    STAGE_MEMTABLE_INSTALL_FLUSH_RESULTS,
                    NUM_OP_STAGES
                };

                enum CompactionPropertyType : int {
                    COMPACTION_JOB_ID = 0,
                    COMPACTION_INPUT_OUTPUT_LEVEL,
                    COMPACTION_PROP_FLAGS,
                    COMPACTION_TOTAL_INPUT_BYTES,
                    COMPACTION_BYTES_READ,
                    COMPACTION_BYTES_WRITTEN,
                    NUM_COMPACTION_PROPERTIES
                };

                enum FlushPropertyType : int {
                    FLUSH_JOB_ID = 0,
                    FLUSH_BYTES_MEMTABLES,
                    FLUSH_BYTES_WRITTEN,
                    NUM_FLUSH_PROPERTIES
                };

                // The maximum number of properties of an operation.
                // This number should be set to the biggest NUM_XXX_PROPERTIES.
                static const int kNumOperationProperties =
                    constexpr_max<NUM_COMPACTION_PROPERTIES, NUM_FLUSH_PROPERTIES>::result;

                enum StateType : int {
                    STATE_UNKNOWN = 0,
                    STATE_MUTEX_WAIT = 1,
                    NUM_STATE_TYPES
                };

        };
        


    } // namespace latte
    
} // namespace latte

#endif