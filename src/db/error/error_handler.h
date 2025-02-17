




#ifndef __LATTE_C_PLUS_ERROR_HANDLER
#define __LATTE_C_PLUS_ERROR_HANDLER
#include "../rocksdb.h"
#include <memory>
#include <thread>
namespace latte
{
    namespace rocksdb
    {
        class RocksDB;
        class ErrorHandler {
            public:
                void EnableAutoRecovery() { auto_recovery_ = true; }
            private:
                RocksDB* db_;
                const ImmutableDBOptions& db_options_;
                Status bg_error_;
                // A separate Status variable used to record any errors during the
                // recovery process from hard errors
                
                //IOStatus recovery_error_;
                
                // The condition variable used with db_mutex during auto resume for time
                // wait.
                //InstrumentedCondVar cv_;
                bool end_recovery_;
                std::unique_ptr<std::thread> recovery_thread_;

                // InstrumentedMutex* db_mutex_;
                // A flag indicating whether automatic recovery from errors is enabled. Auto
                // recovery applies for delegating to SstFileManager to handle no space type
                // of errors. This flag doesn't control the auto resume behavior to recover
                // from retryable IO errors.
                bool auto_recovery_;
                bool recovery_in_prog_;
                // A flag to indicate that for the soft error, we should not allow any
                // background work except the work is from recovery.
                bool soft_error_no_bg_work_;

                // Used to store the context for recover, such as flush reason.
                // DBRecoverContext recover_context_;
                std::atomic<bool> is_db_stopped_;

                // The pointer of DB statistics.
                // std::shared_ptr<Statistics> bg_error_stats_;

                // During recovery from manifest IO errors, files whose VersionEdits entries
                // could be in an ambiguous state are quarantined and file deletion refrain
                // from deleting them. Successful recovery will clear this vector. Files are
                // added to this vector while DB mutex was locked, this data structure is
                // unsorted.
                // autovector<uint64_t> files_to_quarantine_;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif