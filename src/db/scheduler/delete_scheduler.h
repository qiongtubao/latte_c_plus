


#ifndef __LATTE_C_PLUS_DELETE_SCHEDULER_H
#define __LATTE_C_PLUS_DELETE_SCHEDULER_H

#include "sst/sst_file_manager_impl.h"
#include "system/system_clock.h"
#include "io/file_system.h"
#include "log/logging.h"
#include "io/file_system.h"
#include "system/system_clock.h"
#include "instrumented/instrumented_mutex_lock.h"
#include "instrumented/instrumented_cond_var.h"


namespace latte
{
    namespace rocksdb
    {
        class DeleteScheduler {
            public:
                DeleteScheduler(SystemClock* clock, FileSystem* fs,
                  int64_t rate_bytes_per_sec, latte::log::Logger* info_log,
                  SstFileManagerImpl* sst_file_manager,
                  double max_trash_db_ratio, uint64_t bytes_max_delete_chunk);

                ~DeleteScheduler();

                void SetStatisticsPtr(const std::shared_ptr<Statistics>& stats) {
                    InstrumentedMutexLock l(&mu_);
                    stats_ = stats;
                }
            public:
                // 检查路径中是否有任何 .trash 文件，并安排删除它们
                // 如果 sst_file_manager 为 nullptr，则立即删除
                static Status CleanupDirectory(Env* env, SstFileManagerImpl* sfm,
                                                const std::string& path);

            private:
                SystemClock* clock_;
                FileSystem* fs_;

                // total size of trash files
                std::atomic<uint64_t> total_trash_size_;
                // Maximum number of bytes that should be deleted per second
                std::atomic<int64_t> rate_bytes_per_sec_;
                // Mutex to protect queue_, pending_files_, next_trash_bucket_,
                // pending_files_in_buckets_, bg_errors_, closing_, stats_
                InstrumentedMutex mu_;


                struct FileAndDir {
                    FileAndDir(const std::string& _fname, const std::string& _dir,
                            bool _accounted, std::optional<int32_t> _bucket)
                        : fname(_fname), dir(_dir), accounted(_accounted), bucket(_bucket) {}
                    std::string fname;
                    std::string dir;  // empty will be skipped.
                    bool accounted;
                    std::optional<int32_t> bucket;
                };
                // Queue of trash files that need to be deleted
                std::queue<FileAndDir> queue_;
                // Number of trash files that are waiting to be deleted
                int32_t pending_files_;
                // Next trash bucket that can be created
                int32_t next_trash_bucket_;
                // A mapping from trash bucket to number of pending files in the bucket
                std::map<int32_t, int32_t> pending_files_in_buckets_;
                uint64_t bytes_max_delete_chunk_;
                // Errors that happened in BackgroundEmptyTrash (file_path => error)
                std::map<std::string, Status> bg_errors_;

                bool num_link_error_printed_ = false;
                // Set to true in ~DeleteScheduler() to force BackgroundEmptyTrash to stop
                bool closing_;
                // Condition variable signaled in these conditions
                //    - pending_files_ value change from 0 => 1
                //    - pending_files_ value change from 1 => 0
                //    - a value in pending_files_in_buckets change from 1 => 0
                //    - closing_ value is set to true
                InstrumentedCondVar cv_;
                // Background thread running BackgroundEmptyTrash
                std::unique_ptr<port::Thread> bg_thread_;
                // Mutex to protect threads from file name conflicts
                InstrumentedMutex file_move_mu_;
                latte::log::Logger* info_log_;
                SstFileManagerImpl* sst_file_manager_;
                // If the trash size constitutes for more than this fraction of the total DB
                // size we will start deleting new files passed to DeleteScheduler
                // immediately
                // Unaccounted files passed for deletion will not cause change in
                // total_trash_size_ or affect the DeleteScheduler::total_trash_size_ over
                // SstFileManager::total_size_ ratio. Their slow deletion is not subject to
                // this configured threshold either.
                std::atomic<double> max_trash_db_ratio_;
                static const uint64_t kMicrosInSecond = 1000 * 1000LL;
                std::shared_ptr<Statistics> stats_;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif