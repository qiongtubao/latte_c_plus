


#ifndef __LATTE_C_PLUS_SST_FILE_MANAGER_IMPL_H
#define __LATTE_C_PLUS_SST_FILE_MANAGER_IMPL_H

#include "sst_file_manager.h"
#include <memory>
#include "system/system_clock.h"
#include "io/file_system.h"
#include "log/logging.h"
#include "statistics/statistics.h"
#include "port/port_posix.h"
#include "scheduler/delete_scheduler.h"

namespace latte
{
    namespace rocksdb
    {
        class SstFileManagerImpl : public SstFileManager {
            public:
                explicit SstFileManagerImpl(const std::shared_ptr<SystemClock>& clock,
                                            const std::shared_ptr<FileSystem>& fs,
                                            const std::shared_ptr<latte::log::Logger>& logger,
                                            int64_t rate_bytes_per_sec,
                                            double max_trash_db_ratio,
                                            uint64_t bytes_max_delete_chunk);

                void SetStatisticsPtr(const std::shared_ptr<Statistics>& stats) override {
                    stats_ = stats;
                    delete_scheduler_.SetStatisticsPtr(stats);
                }

                // Called by each DB instance using this sst file manager to reserve
                // disk buffer space for recovery from out of space errors
                void ReserveDiskBuffer(uint64_t buffer, const std::string& path);
            public:
                std::shared_ptr<SystemClock> clock_;
                std::shared_ptr<FileSystem> fs_;
                std::shared_ptr<latte::log::Logger> logger_;

                port::Mutex mu_;
                // The summation of the sizes of all files in tracked_files_ map
                uint64_t total_files_size_;
                // Compactions should only execute if they can leave at least
                // this amount of buffer space for logs and flushes
                uint64_t compaction_buffer_size_;
                // Estimated size of the current ongoing compactions
                uint64_t cur_compactions_reserved_size_;
                // A map containing all tracked files and there sizes
                //  file_path => file_size
                std::unordered_map<std::string, uint64_t> tracked_files_;
                // The maximum allowed space (in bytes) for sst and blob files.
                uint64_t max_allowed_space_;
                // DeleteScheduler used to throttle file deletion.
                DeleteScheduler delete_scheduler_;
                port::CondVar cv_;
                // Flag to force error recovery thread to exit
                bool closing_;
                // Background error recovery thread
                std::unique_ptr<port::Thread> bg_thread_;
                // A path in the filesystem corresponding to this SFM. This is used for
                // calling Env::GetFreeSpace. Posix requires a path in the filesystem
                std::string path_;
                // Save the current background error
                Status bg_err_;
                // Amount of free disk headroom before allowing recovery from hard errors
                uint64_t reserved_disk_buffer_;
                // For soft errors, amount of free disk space before we can allow
                // compactions to run full throttle. If disk space is below this trigger,
                // compactions will be gated by free disk space > input size
                uint64_t free_space_trigger_;
                // List of database error handler instances tracked by this SstFileManager.
                std::list<ErrorHandler*> error_handler_list_;
                // Pointer to ErrorHandler instance that is currently processing recovery
                ErrorHandler* cur_instance_;
                std::shared_ptr<Statistics> stats_;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif