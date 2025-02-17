
#ifndef __LATTE_C_PLUS_VERSION_SET_H
#define __LATTE_C_PLUS_VERSION_SET_H

#include <string>
#include "../options/open_options.h"
#include "column_family/column_family_mem_tables.h"
#include "io/fs_directory.h"
#include "wal_edit.h"
#include "error/error_handler.h"

namespace latte
{
    namespace leveldb
    {
        class VersionSet {
            public:
                VersionSet(const std::string& dbname, const OpenOptions* options,
                    TableCache* table_cache, const InternalKeyComparator*);
                
                
        };
    }; // namespace leveldb

    enum EpochNumberRequirement {
        kMightMissing,
        kMustPresent,
    };
    namespace rocksdb {

        class VersionSet {

            public: //static method 获取当前清单路径
                static Status GetCurrentManifestPath(const std::string& dbname,
                                       FileSystem* fs, bool is_retry,
                                       std::string* manifest_filename,
                                       uint64_t* manifest_file_number);
            public:
                VersionSet(const VersionSet&) = delete;
                virtual ~VersionSet();

                void operator=(const VersionSet&) = delete;
                // 从持久存储中恢复最后保存的描述符 (MANIFEST)。
                // 如果 read_only == true，则即使某些列族
                // 未打开，Recover() 也不会报错
                Status Recover(const std::vector<ColumnFamilyDescriptor>& column_families,
                                bool read_only = false, std::string* db_id = nullptr,
                                bool no_error_if_files_missing = false, bool is_retry = false,
                                Status* log_status = nullptr);

                // 从所有可用的 MANIFEST 文件中尽最大努力恢复（Options.best_efforts_recovery=true）。与 `Recover` 类似，但有以下区别：
                // 1) 不仅可以使用最新的 MANIFEST，如果不可用或
                // 无法成功恢复，此函数还会尝试
                // 从以前的 MANIFEST 文件中恢复，按时间顺序倒序
                // 直到成功恢复。
                // 2) 此函数的目的不仅仅是恢复到最新版本，如果
                // 不可用，则最新的时间点版本将保存在
                // 内存中。查看 `VersionEditHandlerPointInTime` 文档以了解更多详细信息。
                Status TryRecover(const std::vector<ColumnFamilyDescriptor>& column_families,
                                    bool read_only,
                                    const std::vector<std::string>& files_in_dbname,
                                    std::string* db_id, bool* has_missing_table_file);

                ColumnFamilySet* GetColumnFamilySet() { return column_family_set_.get(); }

                virtual Status Close(FSDirectory* db_dir, InstrumentedMutex* mu);



                // The returned WalSet needs to be accessed with DB mutex held.
                 const WalSet& GetWalSet() const { return wals_; }
                
                // Status LogAndApplyToDefaultColumnFamily(
                //     const ReadOptions& read_options, const WriteOptions& write_options,
                //     VersionEdit* edit, InstrumentedMutex* mu,
                //     FSDirectory* dir_contains_current_file, bool new_descriptor_log = false,
                //     const ColumnFamilyOptions* column_family_options = nullptr);
                  
                // Allocate and return a new file number
                uint64_t NewFileNumber() { return next_file_number_.fetch_add(1); }
            public:
                // Protected by DB mutex.
                WalSet wals_;
                std::unique_ptr<ColumnFamilySet> column_family_set_;
                // Cache* table_cache_;
                Env* const env_;
                FileSystemPtr const fs_;
                SystemClock* const clock_;
                const std::string dbname_;
                std::string db_id_;
                const ImmutableDBOptions* const db_options_;
                std::atomic<uint64_t> next_file_number_;
                // Any WAL number smaller than this should be ignored during recovery,
                // and is qualified for being deleted.
                std::atomic<uint64_t> min_log_number_to_keep_ = {0};
                uint64_t manifest_file_number_;
                uint64_t options_file_number_;
                uint64_t options_file_size_;
                uint64_t pending_manifest_file_number_;
                // The last seq visible to reads. It normally indicates the last sequence in
                // the memtable but when using two write queues it could also indicate the
                // last sequence in the WAL visible to reads.
                std::atomic<uint64_t> last_sequence_;
                // The last sequence number of data committed to the descriptor (manifest
                // file).
                SequenceNumber descriptor_last_sequence_ = 0;
                // The last seq that is already allocated. It is applicable only when we have
                // two write queues. In that case seq might or might not have appreated in
                // memtable but it is expected to appear in the WAL.
                // We have last_sequence <= last_allocated_sequence_
                std::atomic<uint64_t> last_allocated_sequence_;
                // The last allocated sequence that is also published to the readers. This is
                // applicable only when last_seq_same_as_publish_seq_ is not set. Otherwise
                // last_sequence_ also indicates the last published seq.
                // We have last_sequence <= last_published_sequence_ <=
                // last_allocated_sequence_
                std::atomic<uint64_t> last_published_sequence_;
                uint64_t prev_log_number_;  // 0 or backing store for memtable being compacted

                // Opened lazily
                std::unique_ptr<latte::rocksdb::LogWriter> descriptor_log_;

                // generates a increasing version number for every new version
                uint64_t current_version_number_;

                // Queue of writers to the manifest file
                // std::deque<ManifestWriter*> manifest_writers_;

                // Current size of manifest file
                uint64_t manifest_file_size_;

                // Obsolete files, or during DB shutdown any files not referenced by what's
                // left of the in-memory LSM state.
                // std::vector<ObsoleteFileInfo> obsolete_files_;
                // std::vector<ObsoleteBlobFileInfo> obsolete_blob_files_;
                std::vector<std::string> obsolete_manifests_;

                // env options for all reads and writes except compactions
                FileOptions file_options_;

                // BlockCacheTracer* const block_cache_tracer_;

                // Store the IO status when Manifest is written
                IOStatus io_status_;

                std::shared_ptr<IOTracer> io_tracer_;

                std::string db_session_id_;

                // Off-peak time option used for compaction scoring
                // OffpeakTimeOption offpeak_time_option_;

                // Pointer to the DB's ErrorHandler.
                ErrorHandler* const error_handler_;

            private:               

                const bool read_only_;
                bool closed_;
        };   
    };// namespace rocksdb
    
} // namespace latte


#endif