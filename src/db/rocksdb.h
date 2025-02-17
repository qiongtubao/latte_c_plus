

#ifndef __LATTE_C_PLUS_ROCKSDB_H
#define __LATTE_C_PLUS_ROCKSDB_H

#include "db.h"
#include "options/write_options.h"
#include "options/immutable_db_options.h"
#include "error/error_handler.h"
#include "mutex/instrumented_mutex.h"
#include "mutex/cache_aligned_instrumented_mutex.h"
#include "env/file_lock.h"
#include "io/file_system_tracer.h"
#include "shared/directories.h"
#include "io/file_options.h"
#include "io/data_structure.h"
#include "io/io_tracer.h"
#include "./context/recovery_context.h"
#include "./column_family/column_family_descriptor.h"
#include "./column_family/column_family_handle.h"
#include "version/version_set.h"
#include "version/version_edit.h"
#include <memory>
#include "version/version.h"
#include "./column_family/column_family_handle.h"
#include "instrumented/instrumented_cond_var.h"
#include <set>
#include <list>
#include <limits>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include "log/log_file_number_size.h"
#include "log/log_writer_number.h"
#include "version/super_version.h"

namespace latte
{
    namespace rocksdb
    {
        class Arena;
        class ArenaWrappedDbIter;
        class InMemoryStatsHistoryIterator;
        class PersistentStatsHistoryIterator;
        class TableCache;
        class TaskLimiterToken;
        class Version;
        class VersionEdit;
        class VersionSet;
        class WriteCallback;
        struct JobContext;
        struct ExternalSstFileInfo;
        struct MemTableInfo;

        class RocksDB : public DB {
            public:
                static Status Open(const RocksdbOpenOptions& options, const std::string& dbname, DB** dbptr);
                static Status Open(const OpenOptions& db_options, 
                    const std::string& dbname, 
                    const std::vector<ColumnFamilyDescriptor>& column_families,
                    std::vector<ColumnFamilyHandle*>* handles,
                    DB** dbptr);
                
                static Status Open(const DBOptions& db_options, const std::string& dbname,
                    const std::vector<ColumnFamilyDescriptor>& column_families,
                    std::vector<ColumnFamilyHandle*>* handles, DB** dbptr,
                    const bool seq_per_batch, const bool batch_per_txn,
                    const bool is_retry, bool* can_retry);


                // static Status Open(const OpenOptions& options, const std::string& name, DB** dbptr);
                // static Status Open(const DBOptions& db_options, const std::string& dbname,
                //     const std::vector<ColumnFamilyDescriptor>& column_families,
                //     std::vector<ColumnFamilyHandle*>* handles, DB** dbptr,
                //     const bool seq_per_batch, const bool batch_per_txn,
                //     const bool is_retry, bool* can_retry);

                virtual Status Recover(
                    const std::vector<ColumnFamilyDescriptor>& column_families,
                    bool read_only = false, bool error_if_wal_file_exists = false,
                    bool error_if_data_exists_in_wals = false, bool is_retry = false,
                    uint64_t* recovered_seq = nullptr,
                    RecoveryContext* recovery_ctx = nullptr, bool* can_retry = nullptr);
                virtual Status SetupDBId(const WriteOptions& write_options, bool read_only,
                         bool is_new_db, bool is_retry,
                         VersionEdit* version_edit);
                virtual Status GetDbIdentityFromIdentityFile(const IOOptions& opts,
                                             std::string* identity) const;

                virtual void SetDBId(std::string&& id, bool read_only,
                    VersionEdit* version_edit);         
                virtual Status NewDB(std::vector<std::string>* new_filenames);

                static Status ValidateOptions(const DBOptions& db_options);
                static Status ValidateOptions(
                    const DBOptions& db_options,
                    const std::vector<ColumnFamilyDescriptor>& column_families);
                const Status CreateArchivalDirectory();

                RocksDB(const DBOptions& options, const std::string& dbname,
                    const bool seq_per_batch = false, const bool batch_per_txn = true,
                    bool read_only = false);
                RocksDB(const RocksDB&) = delete;
                void operator=(const RocksDB&) = delete;

                Status MaybeUpdateNextFileNumber(RecoveryContext* recovery_ctx);

                virtual Status CheckConsistency();

                // REQUIRES: log_numbers are sorted in ascending order
                // corrupted_log_found is set to true if we recover from a corrupted log file.
                Status RecoverLogFiles(const std::vector<uint64_t>& log_numbers,
                                        SequenceNumber* next_sequence, bool read_only,
                                        bool is_retry, bool* corrupted_log_found,
                                        RecoveryContext* recovery_ctx);

                const std::string& GetName() const ;

                size_t GetWalPreallocateBlockSize(uint64_t write_buffer_size) const;

                IOStatus CreateWAL(const WriteOptions& write_options, uint64_t log_file_num,
                     uint64_t recycle_log_number, size_t preallocate_block_size,
                     latte::rocksdb::LogWriter** new_log);

                IOStatus WriteToWAL(const WriteBatch& merged_batch,
                      const WriteOptions& write_options,
                      LogWriter* log_writer, uint64_t* log_used,
                      uint64_t* log_size,
                      LogFileNumberSize& log_file_number_size);

                IOStatus WriteToWAL(const WriteThread::WriteGroup& write_group,
                                    LogWriter* log_writer, uint64_t* log_used,
                                    bool need_log_sync, bool need_log_dir_sync,
                                    SequenceNumber sequence,
                                    LogFileNumberSize& log_file_number_size);

                Status FlushWAL(bool sync) {
                    // TODO: plumb Env::IOActivity, Env::IOPriority
                    return FlushWAL(WriteOptions(), sync);
                }

                virtual Status FlushWAL(const WriteOptions& write_options, bool sync);
                bool WALBufferIsEmpty();
                Status SyncWAL() ;
                Status LockWAL() ;
                Status UnlockWAL() ;

                // recovery_ctx 存储有关版本编辑的上下文，并且
                // LogAndApplyForRecovery 在成功同步新 WAL 后将所有这些编辑保留到新 Manifest 中。
                // LogAndApplyForRecovery 在恢复期间应仅被调用一次，并且
                // 应在 RocksDB 写入自此恢复以来的第一个新 MANIFEST 时调用。
                Status LogAndApplyForRecovery(const RecoveryContext& recovery_ctx);

                // 初始化内置列族以用于持久统计信息。根据
                // 之前是否启用过磁盘持久统计信息，它可能
                // 创建新的列族和列族句柄，或者只是创建一个列族
                // 句柄。
                // 必需：DB 互斥锁已持有
                Status InitPersistStatsColumnFamily();

                void NewThreadStatusCfInfo(ColumnFamilyData* cfd) const;

                // 创建列族，尚不需要进行一些后续工作
                Status CreateColumnFamilyImpl(const ReadOptions& read_options,
                                                const WriteOptions& write_options,
                                                const ColumnFamilyOptions& cf_options,
                                                const std::string& cf_name,
                                                ColumnFamilyHandle** handle);

                // 后台线程调用此函数，它只是 InstallSuperVersion() 函数的包装器。
                // 后台线程携带 sv_context，其中可能已经分配了 new_superversion。
                // 所有 ColumnFamily 状态更改都通过此函数。在这里，我们分析新状态，
                // 如果检测到新状态需要刷新或压缩，我们将安排后台工作。
                void InstallSuperVersionAndScheduleWork(
                    ColumnFamilyData* cfd, SuperVersionContext* sv_context,
                    const MutableCFOptions& mutable_cf_options);

                // 持久统计列族有两个格式版本键，用于
                // 兼容性检查。如果是首次创建，则写入格式版本
                // ；如果从磁盘恢复，则读取格式版本并检查兼容性
                // 。此函数需要在入口处持有 DB 互斥锁，但可能
                // 在此过程中释放并重新获取 DB 互斥锁。
                // 必需：持有 DB 互斥锁
                Status PersistentStatsProcessFormatVersion();

                // 将选项保留到选项文件中。必须持有 options_mutex_。
                // 如果 !db_mutex_already_held，将锁定 DB 互斥锁。
                Status WriteOptionsFile(const WriteOptions& write_options,
                                        bool db_mutex_already_held);

                // 跟踪现有数据文件，包括引用和未引用的 SST
                // 和 SstFileManager 中的 Blob 文件。这仅在 DB::Open 期间调用，并且
                // 在任何文件删除开始之前调用，以便可以
                // 正确地限制它们的删除速率。
                // 文件可能未在 MANIFEST 中引用，因为（例如
                // 1. 这是尽力而为的恢复；
                // 2. 引用 SST 文件的 VersionEdits 附加到
                // RecoveryContext，DB 在同步 MANIFEST 时崩溃，VersionEdits
                // 在恢复期间仍未同步到 MANIFEST。）
                //
                // 如果文件在 Manifest 中被引用（通常是绝大多数文件），由于它已经记录了文件大小，因此我们不需要查询文件系统。否则，我们将查询
                // 文件系统以获取未引用文件的大小。
                // 要求：互斥体解锁
                void TrackExistingDataFiles(
                    const std::vector<std::string>& existing_data_files);

                // 收集此 DB 使用的重复数据删除路径集合，包括
                // dbname_、DBOptions.db_paths、ColumnFamilyOptions.cf_paths。
                std::set<std::string> CollectAllDBPaths();

                // 删除任何不需要的文件和陈旧的内存条目。
                void DeleteObsoleteFiles();

                void MaybeScheduleFlushOrCompaction();

                // Schedule background tasks
                Status StartPeriodicTaskScheduler();

                // Cancel scheduled periodic tasks
                Status CancelPeriodicTaskScheduler();

                Status RegisterRecordSeqnoTimeWorker(const ReadOptions& read_options,
                                       const WriteOptions& write_options,
                                       bool is_new_db);
            public:
                    const std::string dbname_;
                    // TODO(peterd): unify with VersionSet::db_id_
                    std::string db_id_;
                    // db_session_id_ is an identifier that gets reset
                    // every time the DB is opened
                    std::string db_session_id_;
                    std::unique_ptr<VersionSet> versions_;
                    // 标记以检查我们是否分配并拥有信息日志文件
                    bool own_info_log_;
                    Status init_logger_creation_s_; //初始化日志记录器创建 返回的状态
                    const DBOptions initial_db_options_;
                    Env* const env_;
                    std::shared_ptr<IOTracer> io_tracer_;
                    const ImmutableDBOptions immutable_db_options_;
                    FileSystemPtr fs_;
                    // MutableDBOptions mutable_db_options_;
                    Statistics* stats_;
                    // std::unordered_map<std::string, RecoveredTransaction*>
                    //     recovered_transactions_;
                    // std::unique_ptr<Tracer> tracer_;
                    // InstrumentedMutex trace_mutex_;
                    // BlockCacheTracer block_cache_tracer_;

                    // constant false canceled flag, used when the compaction is not manual
                    const std::atomic<bool> kManualCompactionCanceledFalse_{false};

                    // State below is protected by mutex_
                    // With two_write_queues enabled, some of the variables that accessed during
                    // WriteToWAL need different synchronization: log_empty_, alive_log_files_,
                    // logs_, logfile_number_. Refer to the definition of each variable below for
                    // more description.
                    //
                    // `mutex_` can be a hot lock in some workloads, so it deserves dedicated
                    // cachelines.
                    // mutable 
                    CacheAlignedInstrumentedMutex mutex_;

                    ColumnFamilyHandleImpl* default_cf_handle_;
                    InternalStats* default_cf_internal_stats_;

                    // table_cache_ provides its own synchronization
                    // std::shared_ptr<Cache> table_cache_;

                    ErrorHandler error_handler_;

                    // Unified interface for logging events
                    // EventLogger event_logger_;

                    // only used for dynamically adjusting max_total_wal_size. it is a sum of
                    // [write_buffer_size * max_write_buffer_number] over all column families
                    std::atomic<uint64_t> max_total_in_memory_state_;

                    // The options to access storage files
                    const FileOptions file_options_;

                    // Additonal options for compaction and flush
                    // FileOptions file_options_for_compaction_;

                    std::unique_ptr<ColumnFamilyMemTablesImpl> column_family_memtables_;

                    // Increase the sequence number after writing each batch, whether memtable is
                    // disabled for that or not. Otherwise the sequence number is increased after
                    // writing each key into memtable. This implies that when disable_memtable is
                    // set, the seq is not increased at all.
                    //
                    // Default: false
                    const bool seq_per_batch_;
                    // This determines during recovery whether we expect one writebatch per
                    // recovered transaction, or potentially multiple writebatches per
                    // transaction. For WriteUnprepared, this is set to false, since multiple
                    // batches can exist per transaction.
                    //
                    // Default: true
                    const bool batch_per_txn_;

                    // Each flush or compaction gets its own job id. this counter makes sure
                    // they're unique
                    std::atomic<int> next_job_id_;

                    std::atomic<bool> shutting_down_;

                    // No new background jobs can be queued if true. This is used to prevent new
                    // background jobs from being queued after WaitForCompact() completes waiting
                    // all background jobs then attempts to close when close_db_ option is true.
                    bool reject_new_background_jobs_;

                    // MutableDBOptions mutable_db_options_;
                    // Statistics* stats_;
                    // std::unordered_map<std::string, RecoveredTransaction*>
                    //     recovered_transactions_;
                    // std::unique_ptr<Tracer> tracer_;
                    // InstrumentedMutex trace_mutex_;
                    // BlockCacheTracer block_cache_tracer_;

                    // Lock over the persistent DB state.  Non-nullptr iff successfully acquired.
                    FileLock* db_lock_;

                    // Guards changes to DB and CF options to ensure consistency between
                    // * In-memory options objects
                    // * Settings in effect
                    // * Options file contents
                    // while allowing the DB mutex to be released during slow operations like
                    // persisting options file or modifying global periodic task timer.
                    // Always acquired *before* DB mutex when this one is applicable.
                    InstrumentedMutex options_mutex_;

                    // Guards reads and writes to in-memory stats_history_.
                    InstrumentedMutex stats_history_mutex_;

                    // In addition to mutex_, log_write_mutex_ protects writes to logs_ and
                    // logfile_number_. With two_write_queues it also protects alive_log_files_,
                    // and log_empty_. Refer to the definition of each variable below for more
                    // details.
                    // Note: to avoid deadlock, if needed to acquire both log_write_mutex_ and
                    // mutex_, the order should be first mutex_ and then log_write_mutex_.
                    InstrumentedMutex log_write_mutex_;

                    // If zero, manual compactions are allowed to proceed. If non-zero, manual
                    // compactions may still be running, but will quickly fail with
                    // `Status::Incomplete`. The value indicates how many threads have paused
                    // manual compactions. It is accessed in read mode outside the DB mutex in
                    // compaction code paths.
                    std::atomic<int> manual_compaction_paused_;

                    // This condition variable is signaled on these conditions:
                    // * whenever bg_compaction_scheduled_ goes down to 0
                    // * if AnyManualCompaction, whenever a compaction finishes, even if it hasn't
                    // made any progress
                    // * whenever a compaction made any progress
                    // * whenever bg_flush_scheduled_ or bg_purge_scheduled_ value decreases
                    // (i.e. whenever a flush is done, even if it didn't make any progress)
                    // * whenever there is an error in background purge, flush or compaction
                    // * whenever num_running_ingest_file_ goes to 0.
                    // * whenever pending_purge_obsolete_files_ goes to 0.
                    // * whenever disable_delete_obsolete_files_ goes to 0.
                    // * whenever SetOptions successfully updates options.
                    // * whenever a column family is dropped.
                    InstrumentedCondVar bg_cv_;
                    // Writes are protected by locking both mutex_ and log_write_mutex_, and reads
                    // must be under either mutex_ or log_write_mutex_. Since after ::Open,
                    // logfile_number_ is currently updated only in write_thread_, it can be read
                    // from the same write_thread_ without any locks.
                    uint64_t logfile_number_;
                    // Log files that we can recycle. Must be protected by db mutex_.
                    std::deque<uint64_t> log_recycle_files_;
                    // The minimum log file number taht can be recycled, if log recycling is
                    // enabled. This is used to ensure that log files created by previous
                    // instances of the database are not recycled, as we cannot be sure they
                    // were created in the recyclable format.
                    uint64_t min_log_number_to_recycle_;
                    // Protected by log_write_mutex_.
                    bool log_dir_synced_;
                    // Without two_write_queues, read and writes to log_empty_ are protected by
                    // mutex_. Since it is currently updated/read only in write_thread_, it can be
                    // accessed from the same write_thread_ without any locks. With
                    // two_write_queues writes, where it can be updated in different threads,
                    // read and writes are protected by log_write_mutex_ instead. This is to avoid
                    // expensive mutex_ lock during WAL write, which update log_empty_.
                    bool log_empty_;

                    ColumnFamilyHandleImpl* persist_stats_cf_handle_;

                    bool persistent_stats_cfd_exists_ = true;

                    // The current WAL file and those that have not been found obsolete from
                    // memtable flushes. A WAL not on this list might still be pending writer
                    // flush and/or sync and close and might still be in logs_. alive_log_files_
                    // is protected by mutex_ and log_write_mutex_ with details as follows:
                    // 1. read by FindObsoleteFiles() which can be called in either application
                    //    thread or RocksDB bg threads, both mutex_ and log_write_mutex_ are
                    //    held.
                    // 2. pop_front() by FindObsoleteFiles(), both mutex_ and log_write_mutex_
                    //    are held.
                    // 3. push_back() by DBImpl::Open() and DBImpl::RestoreAliveLogFiles()
                    //    (actually called by Open()), only mutex_ is held because at this point,
                    //    the DB::Open() call has not returned success to application, and the
                    //    only other thread(s) that can conflict are bg threads calling
                    //    FindObsoleteFiles() which ensure that both mutex_ and log_write_mutex_
                    //    are held when accessing alive_log_files_.
                    // 4. read by DBImpl::Open() is protected by mutex_.
                    // 5. push_back() by SwitchMemtable(). Both mutex_ and log_write_mutex_ are
                    //    held. This is done by the write group leader. Note that in the case of
                    //    two-write-queues, another WAL-only write thread can be writing to the
                    //    WAL concurrently. See 9.
                    // 6. read by SwitchWAL() with both mutex_ and log_write_mutex_ held. This is
                    //    done by write group leader.
                    // 7. read by ConcurrentWriteToWAL() by the write group leader in the case of
                    //    two-write-queues. Only log_write_mutex_ is held to protect concurrent
                    //    pop_front() by FindObsoleteFiles().
                    // 8. read by PreprocessWrite() by the write group leader. log_write_mutex_
                    //    is held to protect the data structure from concurrent pop_front() by
                    //    FindObsoleteFiles().
                    // 9. read by ConcurrentWriteToWAL() by a WAL-only write thread in the case
                    //    of two-write-queues. Only log_write_mutex_ is held. This suffices to
                    //    protect the data structure from concurrent push_back() by current
                    //    write group leader as well as pop_front() by FindObsoleteFiles().
                    std::deque<LogFileNumberSize> alive_log_files_;

                    // Log files that aren't fully synced, and the current log file.
                    // Synchronization:
                    // 1. read by FindObsoleteFiles() which can be called either in application
                    //    thread or RocksDB bg threads. log_write_mutex_ is always held, while
                    //    some reads are performed without mutex_.
                    // 2. pop_front() by FindObsoleteFiles() with only log_write_mutex_ held.
                    // 3. read by DBImpl::Open() with both mutex_ and log_write_mutex_.
                    // 4. emplace_back() by DBImpl::Open() with both mutex_ and log_write_mutex.
                    //    Note that at this point, DB::Open() has not returned success to
                    //    application, thus the only other thread(s) that can conflict are bg
                    //    threads calling FindObsoleteFiles(). See 1.
                    // 5. iteration and clear() from CloseHelper() always hold log_write_mutex
                    //    and mutex_.
                    // 6. back() called by APIs FlushWAL() and LockWAL() are protected by only
                    //    log_write_mutex_. These two can be called by application threads after
                    //    DB::Open() returns success to applications.
                    // 7. read by SyncWAL(), another API, protected by only log_write_mutex_.
                    // 8. read by MarkLogsNotSynced() and MarkLogsSynced() are protected by
                    //    log_write_mutex_.
                    // 9. erase() by MarkLogsSynced() protected by log_write_mutex_.
                    // 10. read by SyncClosedWals() protected by only log_write_mutex_. This can
                    //     happen in bg flush threads after DB::Open() returns success to
                    //     applications.
                    // 11. reads, e.g. front(), iteration, and back() called by PreprocessWrite()
                    //     holds only the log_write_mutex_. This is done by the write group
                    //     leader. A bg thread calling FindObsoleteFiles() or MarkLogsSynced()
                    //     can happen concurrently. This is fine because log_write_mutex_ is used
                    //     by all parties. See 2, 5, 9.
                    // 12. reads, empty(), back() called by SwitchMemtable() hold both mutex_ and
                    //     log_write_mutex_. This happens in the write group leader.
                    // 13. emplace_back() by SwitchMemtable() hold both mutex_ and
                    //     log_write_mutex_. This happens in the write group leader. Can conflict
                    //     with bg threads calling FindObsoleteFiles(), MarkLogsSynced(),
                    //     SyncClosedWals(), etc. as well as application threads calling
                    //     FlushWAL(), SyncWAL(), LockWAL(). This is fine because all parties
                    //     require at least log_write_mutex_.
                    // 14. iteration called in WriteToWAL(write_group) protected by
                    //     log_write_mutex_. This is done by write group leader when
                    //     two-write-queues is disabled and write needs to sync logs.
                    // 15. back() called in ConcurrentWriteToWAL() protected by log_write_mutex_.
                    //     This can be done by the write group leader if two-write-queues is
                    //     enabled. It can also be done by another WAL-only write thread.
                    //
                    // Other observations:
                    //  - back() and items with getting_synced=true are not popped,
                    //  - The same thread that sets getting_synced=true will reset it.
                    //  - it follows that the object referred by back() can be safely read from
                    //  the write_thread_ without using mutex. Note that calling back() without
                    //  mutex may be unsafe because different implementations of deque::back() may
                    //  access other member variables of deque, causing undefined behaviors.
                    //  Generally, do not access stl containers without proper synchronization.
                    //  - it follows that the items with getting_synced=true can be safely read
                    //  from the same thread that has set getting_synced=true
                    std::deque<LogWriterNumber> logs_;

                    // Signaled when getting_synced becomes false for some of the logs_.
                    InstrumentedCondVar log_sync_cv_;
                    // This is the app-level state that is written to the WAL but will be used
                    // only during recovery. Using this feature enables not writing the state to
                    // memtable on normal writes and hence improving the throughput. Each new
                    // write of the state will replace the previous state entirely even if the
                    // keys in the two consecutive states do not overlap.
                    // It is protected by log_write_mutex_ when two_write_queues_ is enabled.
                    // Otherwise only the heaad of write_thread_ can access it.
                    // WriteBatch cached_recoverable_state_;
                    std::atomic<bool> cached_recoverable_state_empty_ = {true};
                    std::atomic<uint64_t> total_log_size_;

                    // If this is non-empty, we need to delete these log files in background
                    // threads. Protected by log_write_mutex_.
                    autovector<latte::rocksdb::LogWriter*> logs_to_free_;

                    bool is_snapshot_supported_;

                    std::map<uint64_t, std::map<std::string, uint64_t>> stats_history_;

                    std::map<std::string, uint64_t> stats_slice_;

                    bool stats_slice_initialized_ = false;

                    Directories directories_;

                    // WriteBufferManager* write_buffer_manager_;

                    // WriteThread write_thread_;
                    // WriteBatch tmp_batch_;
                    // The write thread when the writers have no memtable write. This will be used
                    // in 2PC to batch the prepares separately from the serial commit.
                    // WriteThread nonmem_write_thread_;

                    // WriteController write_controller_;

                    // Size of the last batch group. In slowdown mode, next write needs to
                    // sleep if it uses up the quota.
                    // Note: This is to protect memtable and compaction. If the batch only writes
                    // to the WAL its size need not to be included in this.
                    uint64_t last_batch_group_size_;

                    // FlushScheduler flush_scheduler_;

                    // TrimHistoryScheduler trim_history_scheduler_;

                    // SnapshotList snapshots_;

                    // TimestampedSnapshotList timestamped_snapshots_;

                    // For each background job, pending_outputs_ keeps the current file number at
                    // the time that background job started.
                    // FindObsoleteFiles()/PurgeObsoleteFiles() never deletes any file that has
                    // number bigger than any of the file number in pending_outputs_. Since file
                    // numbers grow monotonically, this also means that pending_outputs_ is always
                    // sorted. After a background job is done executing, its file number is
                    // deleted from pending_outputs_, which allows PurgeObsoleteFiles() to clean
                    // it up.
                    // State is protected with db mutex.
                    std::list<uint64_t> pending_outputs_;

                    // Similar to pending_outputs_, FindObsoleteFiles()/PurgeObsoleteFiles() never
                    // deletes any OPTIONS file that has number bigger than any of the file number
                    // in min_options_file_numbers_.
                    std::list<uint64_t> min_options_file_numbers_;

                    // flush_queue_ and compaction_queue_ hold column families that we need to
                    // flush and compact, respectively.
                    // A column family is inserted into flush_queue_ when it satisfies condition
                    // cfd->imm()->IsFlushPending()
                    // A column family is inserted into compaction_queue_ when it satisfied
                    // condition cfd->NeedsCompaction()
                    // Column families in this list are all Ref()-erenced
                    // TODO(icanadi) Provide some kind of ReferencedColumnFamily class that will
                    // do RAII on ColumnFamilyData
                    // Column families are in this queue when they need to be flushed or
                    // compacted. Consumers of these queues are flush and compaction threads. When
                    // column family is put on this queue, we increase unscheduled_flushes_ and
                    // unscheduled_compactions_. When these variables are bigger than zero, that
                    // means we need to schedule background threads for flush and compaction.
                    // Once the background threads are scheduled, we decrease unscheduled_flushes_
                    // and unscheduled_compactions_. That way we keep track of number of
                    // compaction and flush threads we need to schedule. This scheduling is done
                    // in MaybeScheduleFlushOrCompaction()
                    // invariant(column family present in flush_queue_ <==>
                    // ColumnFamilyData::pending_flush_ == true)
                    // std::deque<FlushRequest> flush_queue_;
                    // invariant(column family present in compaction_queue_ <==>
                    // ColumnFamilyData::pending_compaction_ == true)
                    std::deque<ColumnFamilyData*> compaction_queue_;

                    // A map to store file numbers and filenames of the files to be purged
                    // std::unordered_map<uint64_t, PurgeFileInfo> purge_files_;

                    // A vector to store the file numbers that have been assigned to certain
                    // JobContext. Current implementation tracks table and blob files only.
                    std::unordered_set<uint64_t> files_grabbed_for_purge_;

                    // A queue to store log writers to close. Protected by db mutex_.
                    std::deque<rocksdb::LogWriter*> logs_to_free_queue_;

                    std::deque<SuperVersion*> superversions_to_free_queue_;

                    int unscheduled_flushes_;

                    int unscheduled_compactions_;

                    // count how many background compactions are running or have been scheduled in
                    // the BOTTOM pool
                    int bg_bottom_compaction_scheduled_;

                    // count how many background compactions are running or have been scheduled
                    int bg_compaction_scheduled_;

                    // stores the number of compactions are currently running
                    int num_running_compactions_;

                    // number of background memtable flush jobs, submitted to the HIGH pool
                    int bg_flush_scheduled_;

                    // stores the number of flushes are currently running
                    int num_running_flushes_;

                    // number of background obsolete file purge jobs, submitted to the HIGH pool
                    int bg_purge_scheduled_;

                    // std::deque<ManualCompactionState*> manual_compaction_dequeue_;

                    // shall we disable deletion of obsolete files
                    // if 0 the deletion is enabled.
                    // if non-zero, files will not be getting deleted
                    // This enables two different threads to call
                    // EnableFileDeletions() and DisableFileDeletions()
                    // without any synchronization
                    int disable_delete_obsolete_files_;

                    // Number of times FindObsoleteFiles has found deletable files and the
                    // corresponding call to PurgeObsoleteFiles has not yet finished.
                    int pending_purge_obsolete_files_;

                    // last time when DeleteObsoleteFiles with full scan was executed. Originally
                    // initialized with startup time.
                    uint64_t delete_obsolete_files_last_run_;

                    // The thread that wants to switch memtable, can wait on this cv until the
                    // pending writes to memtable finishes.
                    std::condition_variable switch_cv_;
                    // The mutex used by switch_cv_. mutex_ should be acquired beforehand.
                    std::mutex switch_mutex_;
                    // Number of threads intending to write to memtable
                    std::atomic<size_t> pending_memtable_writes_ = {};

                    // A flag indicating whether the current rocksdb database has any
                    // data that is not yet persisted into either WAL or SST file.
                    // Used when disableWAL is true.
                    std::atomic<bool> has_unpersisted_data_;

                    // if an attempt was made to flush all column families that
                    // the oldest log depends on but uncommitted data in the oldest
                    // log prevents the log from being released.
                    // We must attempt to free the dependent memtables again
                    // at a later time after the transaction in the oldest
                    // log is fully commited.
                    bool unable_to_release_oldest_log_;

                    // Number of running IngestExternalFile() or CreateColumnFamilyWithImport()
                    // calls.
                    // REQUIRES: mutex held
                    int num_running_ingest_file_;

                    // WalManager wal_manager_;

                    // A value of > 0 temporarily disables scheduling of background work
                    int bg_work_paused_;

                    // A value of > 0 temporarily disables scheduling of background compaction
                    int bg_compaction_paused_;

                    // Guard against multiple concurrent refitting
                    bool refitting_level_;

                    // Indicate DB was opened successfully
                    bool opened_successfully_;

                    // The min threshold to triggere bottommost compaction for removing
                    // garbages, among all column families.
                    SequenceNumber bottommost_files_mark_threshold_ = kMaxSequenceNumber;

                    // The min threshold to trigger compactions for standalone range deletion
                    // files that are marked for compaction.
                    SequenceNumber standalone_range_deletion_files_mark_threshold_ =
                        kMaxSequenceNumber;

                    // LogsWithPrepTracker logs_with_prep_tracker_;

                    // Callback for compaction to check if a key is visible to a snapshot.
                    // REQUIRES: mutex held
                    // std::unique_ptr<SnapshotChecker> snapshot_checker_;

                    // Callback for when the cached_recoverable_state_ is written to memtable
                    // Only to be set during initialization
                    // std::unique_ptr<PreReleaseCallback> recoverable_state_pre_release_callback_;

                    // Scheduler to run DumpStats(), PersistStats(), and FlushInfoLog().
                    // Currently, internally it has a global timer instance for running the tasks.
                    // PeriodicTaskScheduler periodic_task_scheduler_;

                    // It contains the implementations for each periodic task.
                    // std::map<PeriodicTaskType, const PeriodicTaskFunc> periodic_task_functions_;

                    // When set, we use a separate queue for writes that don't write to memtable.
                    // In 2PC these are the writes at Prepare phase.
                    const bool two_write_queues_;
                    const bool manual_wal_flush_;

                    // LastSequence also indicates last published sequence visibile to the
                    // readers. Otherwise LastPublishedSequence should be used.
                    const bool last_seq_same_as_publish_seq_;
                    // It indicates that a customized gc algorithm must be used for
                    // flush/compaction and if it is not provided vis SnapshotChecker, we should
                    // disable gc to be safe.
                    const bool use_custom_gc_;
                    // Flag to indicate that the DB instance shutdown has been initiated. This
                    // different from shutting_down_ atomic in that it is set at the beginning
                    // of shutdown sequence, specifically in order to prevent any background
                    // error recovery from going on in parallel. The latter, shutting_down_,
                    // is set a little later during the shutdown after scheduling memtable
                    // flushes
                    std::atomic<bool> shutdown_initiated_;
                    // Flag to indicate whether sst_file_manager object was allocated in
                    // DB::Open() or passed to us
                    bool own_sfm_;

                    // Flag to check whether Close() has been called on this DB
                    bool closed_;
                    // save the closing status, for re-calling the close()
                    Status closing_status_;
                    // mutex for DB::Close()
                    InstrumentedMutex closing_mutex_;

                    // Conditional variable to coordinate installation of atomic flush results.
                    // With atomic flush, each bg thread installs the result of flushing multiple
                    // column families, and different threads can flush different column
                    // families. It's difficult to rely on one thread to perform batch
                    // installation for all threads. This is different from the non-atomic flush
                    // case.
                    // atomic_flush_install_cv_ makes sure that threads install atomic flush
                    // results sequentially. Flush results of memtables with lower IDs get
                    // installed to MANIFEST first.
                    InstrumentedCondVar atomic_flush_install_cv_;

                    bool wal_in_db_path_;
                    std::atomic<uint64_t> max_total_wal_size_;

                    // BlobFileCompletionCallback blob_callback_;

                    // Pointer to WriteBufferManager stalling interface.
                    // std::unique_ptr<StallInterface> wbm_stall_;

                    // seqno_to_time_mapping_ stores the sequence number to time mapping, it's not
                    // thread safe, both read and write need db mutex hold.
                    // SeqnoToTimeMapping seqno_to_time_mapping_;

                    // Stop write token that is acquired when first LockWAL() is called.
                    // Destroyed when last UnlockWAL() is called. Controlled by DB mutex.
                    // See lock_wal_count_
                    // std::unique_ptr<WriteControllerToken> lock_wal_write_token_;

                    // The number of LockWAL called without matching UnlockWAL call.
                    // See also lock_wal_write_token_
                    uint32_t lock_wal_count_;
        };

        OpenOptions SanitizeOptions(const std::string& db, const OpenOptions& src,
                        bool read_only = false,
                        Status* logger_creation_s = nullptr);
        DBOptions SanitizeOptions(const std::string& db, const DBOptions& src,
                            bool read_only = false,
                            Status* logger_creation_s = nullptr);
        
    } // namespace rocksdb
    
} // namespace latte

#endif