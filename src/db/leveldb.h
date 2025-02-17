#ifndef __LATTE_C_PLUS_LEVELDB_H
#define __LATTE_C_PLUS_LEVELDB_H

#include "db.h"
#include "port/port_example.h"
#include "version/version_set.h"
#include "format/internal_key_comparator.h"
#include "format/internal_policy.h"
#include <deque>
#include "format/config.h"
#include "env/env.h"
#include "env/file_lock.h"
#include "port/cond_var.h"
#include "batch/write_batch.h"
#include "log/log_writer.h"
#include "snapshot/snapshot.h"
#include "compaction/manual_compaction.h"
#include "compaction/compaction_stats.h"
#include "options/read_options.h"

namespace latte
{
    namespace leveldb
    {
        class MemTable;
        class TableCache;
        class Version;
        class VersionEdit;
        class VersionSet;

         class LevelDB: public DB {
            public:
                class Writer;
                static Status Open(const OpenOptions& options, const std::string& name, DB** dbptr);
                //方法
                LevelDB(const OpenOptions& options, const std::string& dbname);
                
                LevelDB(const LevelDB&) = delete;

                ~LevelDB();

                Status Get(const ReadOptions& options, const Slice& key,
                    std::string* value) override; //重写函数不然认为类是虚类

                Status Recover(VersionEdit* edit, bool* save_manifest)
                    EXCLUSIVE_LOCKS_REQUIRED(mutex_);

                //属性
                Env* const env_;
                const InternalKeyComparator internal_comparator_; //对比
                const InternalFilterPolicy internal_filter_policy_;
                const OpenOptions options_;  // options_.comparator == &internal_comparator_
                const bool owns_info_log_;
                const bool owns_cache_;
                const std::string dbname_;
                // table_cache_ provides its own synchronization
                TableCache* const table_cache_;

                // Lock over the persistent DB state.  Non-null iff successfully acquired.
                FileLock* db_lock_; //文件锁

                // State below is protected by mutex_
                latte::port::Mutex mutex_;
                std::atomic<bool> shutting_down_;
                latte::port::CondVar background_work_finished_signal_ GUARDED_BY(mutex_);
                MemTable* mem_;
                MemTable* imm_ GUARDED_BY(mutex_); // 正在压缩的 Memtable
                // std::atomic<bool> has_imm_;
                // WritableFile* logfile_;
                uint64_t logfile_number_ GUARDED_BY(mutex_);
                // latte::Writer* log_;
                uint32_t seed_ GUARDED_BY(mutex_);  // For sampling.

                // Queue of writers.
                std::deque<Writer*> writers_ GUARDED_BY(mutex_);
                WriteBatch* tmp_batch_ GUARDED_BY(mutex_);

                SnapshotList snapshots_ GUARDED_BY(mutex_);

                // Set of table files to protect from deletion because they are
                // part of ongoing compactions.
                std::set<uint64_t> pending_outputs_ GUARDED_BY(mutex_);

                // Has a background compaction been scheduled or is running?
                bool background_compaction_scheduled_ GUARDED_BY(mutex_);

                ManualCompaction* manual_compaction_ GUARDED_BY(mutex_);

                VersionSet* const versions_ GUARDED_BY(mutex_);

                // Have we encountered a background error in paranoid mode?
                Status bg_error_ GUARDED_BY(mutex_);

                CompactionStats stats_[config::kNumLevels] GUARDED_BY(mutex_);
        };

    } // namespace leveldb
    
    
} // namespace latte

#endif