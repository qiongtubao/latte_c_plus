
#include "leveldb.h"
#include "version/version_edit.h"
#include <assert.h>
#include <set>
namespace latte
{   
    namespace leveldb
    {

        LevelDB::LevelDB(const OpenOptions& options, const std::string& dbname) 
            : 
            // env_(raw_options.env),
            // internal_comparator_(raw_options.comparator),
            // internal_filter_policy_(raw_options.filter_policy),
            // options_(SanitizeOptions(dbname, &internal_comparator_,
            //         &internal_filter_policy_, raw_options)),
            // owns_info_log_(options_.info_log != raw_options.info_log),
            // owns_cache_(options_.block_cache != raw_options.block_cache),
            dbname_(dbname),
            // table_cache_(new TableCache(dbname_, options_, TableCacheSize(options_))),
            // db_lock_(nullptr),
            // shutting_down_(false),
            // background_work_finished_signal_(&mutex_),
            // mem_(nullptr),
            // imm_(nullptr),
            // has_imm_(false),
            // logfile_(nullptr),
            // logfile_number_(0),
            // log_(nullptr),
            // seed_(0),
            // tmp_batch_(new WriteBatch),
            // background_compaction_scheduled_(false),
            // manual_compaction_(nullptr),
            versions_(new VersionSet(dbname_, &options_, table_cache_,
                                    &internal_comparator_))
            {
            

        };

        // Status LevelDB::Get(const ReadOptions& options, const Slice& key, std::string* value) {
            
        // };

        Status LevelDB::Open(const OpenOptions& options, const std::string& dbname, DB** dbptr) {
            *dbptr = nullptr;
            LevelDB* leveldb = new LevelDB(options, dbname);
            leveldb->mutex_.Lock(); //加锁
            VersionEdit edit; //恢复数据用的对象

            bool save_manifest = false;
            Status s = leveldb->Recover(&edit, &save_manifest);  //恢复数据
            if (s.ok() && leveldb->mem_ == nullptr) {
                uint64_t new_log_number = leveldb->versions_->NewFileNumber();
                WritableFile* lfile;
                s = options.env->NewWritableFile(LogFileName(dbname, new_log_number),
                                     &lfile);
                if (s.ok()) {
                    edit.SetLogNumber(new_log_number);
                    leveldb->logfile_ = lfile;
                    leveldb->logfile_number_ = new_log_number;
                    leveldb->log_ = new log::Writer(lfile);
                    leveldb->mem_ = new MemTable(impl->internal_comparator_);
                    leveldb->mem_->Ref();
                }
            }

            if (s.ok() && save_manifest) {
                edit.SetPrevLogNumber(0);  // No older logs needed after recovery.
                edit.SetLogNumber(leveldb->logfile_number_);
                s = leveldb->versions_->LogAndApply(&edit, &leveldb->mutex_);
            }
            if (s.ok()) {
                leveldb->RemoveObsoleteFiles();
                leveldb->MaybeScheduleCompaction();
            }
            leveldb->mutex_.Unlock(); //解锁
            if (s.ok()) {
                assert(leveldb->mem_ != nullptr);
                *dbptr = leveldb;
            } else {
                delete leveldb;
            }
            return s;
        };

        Status LevelDB::Recover(VersionEdit* edit, bool* save_manifest) { //恢复数据
            mutex_.AssertHeld();

            // Ignore error from CreateDir since the creation of the DB is
            // committed only when the descriptor is created, and this directory
            // may already exist from a previous failed creation attempt.
            env_->CreateDir(dbname_);
            assert(db_lock_ == nullptr);
            Status s = env_->LockFile(LockFileName(dbname_), &db_lock_);
            if (!s.ok()) {
                return s;
            }

            if (!env_->FileExists(CurrentFileName(dbname_))) {
                if (options_.create_if_missing) {
                Log(options_.info_log, "Creating DB %s since it was missing.",
                    dbname_.c_str());
                s = NewDB();
                if (!s.ok()) {
                    return s;
                }
                } else {
                return Status::InvalidArgument(
                    dbname_, "does not exist (create_if_missing is false)");
                }
            } else {
                if (options_.error_if_exists) {
                return Status::InvalidArgument(dbname_,
                                                "exists (error_if_exists is true)");
                }
            }

            s = versions_->Recover(save_manifest); //恢复版本数据
            if (!s.ok()) {
                return s;
            }
            SequenceNumber max_sequence(0); //序列

            // Recover from all newer log files than the ones named in the
            // descriptor (new log files may have been added by the previous
            // incarnation without registering them in the descriptor).
            //
            // Note that PrevLogNumber() is no longer used, but we pay
            // attention to it in case we are recovering a database
            // produced by an older version of leveldb.
            const uint64_t min_log = versions_->LogNumber();
            const uint64_t prev_log = versions_->PrevLogNumber();
            std::vector<std::string> filenames;
            s = env_->GetChildren(dbname_, &filenames);
            if (!s.ok()) {
                return s;
            }
            std::set<uint64_t> expected;
            versions_->AddLiveFiles(&expected);
            uint64_t number;
            FileType type;
            std::vector<uint64_t> logs;
            for (size_t i = 0; i < filenames.size(); i++) {
                if (ParseFileName(filenames[i], &number, &type)) {
                expected.erase(number);
                if (type == kLogFile && ((number >= min_log) || (number == prev_log)))
                    logs.push_back(number);
                }
            }
            if (!expected.empty()) {
                char buf[50];
                std::snprintf(buf, sizeof(buf), "%d missing files; e.g.",
                            static_cast<int>(expected.size()));
                return Status::Corruption(buf, TableFileName(dbname_, *(expected.begin())));
            }

            // Recover in the order in which the logs were generated
            std::sort(logs.begin(), logs.end());
            for (size_t i = 0; i < logs.size(); i++) {
                s = RecoverLogFile(logs[i], (i == logs.size() - 1), save_manifest, edit,
                                &max_sequence);
                if (!s.ok()) {
                return s;
                }

                // The previous incarnation may not have written any MANIFEST
                // records after allocating this log number.  So we manually
                // update the file number allocation counter in VersionSet.
                versions_->MarkFileNumberUsed(logs[i]);
            }

            if (versions_->LastSequence() < max_sequence) {
                versions_->SetLastSequence(max_sequence);
            }

            return Status::OK();
        }

    } // namespace leveldb
    
    
} // namespace latte
