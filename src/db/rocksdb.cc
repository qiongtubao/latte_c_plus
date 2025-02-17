


#include "rocksdb.h"
#include "thread/thread_status_util.h"


#include "column_family/advanced_column_family_options.h"
#include "column_family/column_family_handle.h"
#include "column_family/column_family_descriptor.h"
#include "env/env.h"
#include "context/recovery_context.h"
#include "types.h"
#include <assert.h>
#include "filename.h"
#include "io/types.h"
#include "io/random_access.h"
#include "io/file_options.h"
#include "version/version_edit.h"
#include "log/logging.h"
#include "io/read_write_util.h"
#include "./write_thread/writable_file_writer.h"
#include "log/log_writer.h"
#include "file/file_util.h"
#include "./column_family/column_family_mem_tables.h"
#include "options/advanced_options.h"

#include "options/options_helper.h"
#include <cinttypes>

#include "instrumented/instrumented_mutex_lock.h"
#include "batch/write_batch.h"
#include "batch/write_batch_internal.h"
#include "context/job_context.h"
#include "sst/sst_file_manager_impl.h"

namespace latte
{
    namespace rocksdb
    {
        const std::string kDefaultColumnFamilyName("default");
        const std::string kPersistentStatsColumnFamilyName(
            "___rocksdb_stats_history___");
        

        

        //验证参数
        Status RocksDB::ValidateOptions(
            const DBOptions& db_options,
            const std::vector<ColumnFamilyDescriptor>& column_families) {
            Status s;
            for (auto& cfd : column_families) {
                s = ColumnFamilyData::ValidateOptions(db_options, cfd.options);
                if (!s.ok()) {
                return s;
                }
            }
            s = ValidateOptions(db_options);
            return s;
        };

        //验证
        Status ValidateOptionsByTable(
            const DBOptions& db_opts,
            const std::vector<ColumnFamilyDescriptor>& column_families) {
            Status s;
            for (auto& cf : column_families) {
                s = ValidateOptions(db_opts, cf.options);
                if (!s.ok()) {
                    return s;
                }
            }
            return Status::OK();
        }
        
        //从身份文件获取数据库身份
        Status RocksDB::GetDbIdentityFromIdentityFile(const IOOptions& opts,
                                             std::string* identity) const {
            std::string idfilename = IdentityFileName(dbname_);
            const FileOptions soptions;

            Status s = ReadFileToString(fs_.get(), idfilename, opts, identity);
            if (!s.ok()) {
                return s;
            }

            // If last character is '\n' remove it from identity. (Old implementations
            // of Env::GenerateUniqueId() would include a trailing '\n'.)
            if (identity->size() > 0 && identity->back() == '\n') {
                identity->pop_back();
            }
            return s;
        }

        //设置db id
        Status RocksDB::SetupDBId(const WriteOptions& write_options, bool read_only,
                         bool is_new_db, bool is_retry,
                         VersionEdit* version_edit) {
            Status s;
            if (!is_new_db) {
                // 检查 身份 文件，如果不存在则创建它，或者
                // 损坏或不匹配清单
                std::string db_id_in_file;
                //判断身份文件是否存在
                s = fs_->FileExists(IdentityFileName(dbname_), IOOptions(), nullptr);
                if (s.ok()) {
                    IOOptions opts;
                    if (is_retry) {
                        opts.verify_and_reconstruct_read = true;
                    }
                    //从身份文件获取数据库身份
                    s = GetDbIdentityFromIdentityFile(opts, &db_id_in_file);
                    if (s.ok() && !db_id_in_file.empty()) {
                        if (db_id_.empty()) {
                        // 从文件加载，但清单中尚未知晓
                        SetDBId(std::move(db_id_in_file), read_only, version_edit);
                            return s;
                        } else if (db_id_ == db_id_in_file) {
                            // 从文件加载并匹配清单
                            return s;
                        }
                    }
                }
                //不存在
                if (s.IsNotFound()) {
                    s = Status::OK();
                }

                if (!s.ok()) {
                    assert(s.IsIOError());
                    return s;
                }
            }
            // 否则 身份 文件丢失或无效。
            // 如果需要，生成新的 ID
            if (db_id_.empty()) {
                SetDBId(env_->GenerateUniqueId(), read_only, version_edit);
            }
            // 如果允许，则将其保存到 身份 文件中
            if (!read_only && immutable_db_options_.write_identity_file) {
                s = SetIdentityFile(write_options, env_, dbname_,
                                    immutable_db_options_.metadata_write_temperature,
                                    db_id_);
            }
            // 注意：write_identity_file=false 的过时 身份文件将在其他地方处理，
            // 因此只有在成功恢复后才会被删除
            return s;
        }
        
        //创建db
        Status RocksDB::NewDB(std::vector<std::string>* new_filenames) {
            VersionEdit new_db_edit;
            const WriteOptions write_options(Env::IOActivity::kDBOpen);
            //获得dbid
            Status s = SetupDBId(write_options, /*read_only=*/false, /*is_new_db=*/true,
                                /*is_retry=*/false, &new_db_edit);
            if (!s.ok()) {
                return s;
            }
            
            new_db_edit.SetLogNumber(0);
            new_db_edit.SetNextFile(2); 
            //设置最后序列号
            new_db_edit.SetLastSequence(0);

            ROCKS_LOG_INFO(immutable_db_options_.info_log, "Creating manifest 1 \n");
            //创建描述符文件名  
            const std::string manifest = DescriptorFileName(dbname_, 1);
            {
                //文件存在
                if (fs_->FileExists(manifest, IOOptions(), nullptr).ok()) {
                    //删除文件
                    fs_->DeleteFile(manifest, IOOptions(), nullptr).PermitUncheckedError();
                }
                //写入文件
                std::unique_ptr<FSWritableFile> file;
                //获得manifest 写入配置
                FileOptions file_options = fs_->OptimizeForManifestWrite(file_options_);
                // 当 kUnknown 不为真时，DB 选项优先
                if (immutable_db_options_.metadata_write_temperature !=
                    Temperature::kUnknown) {
                    file_options.temperature =
                        immutable_db_options_.metadata_write_temperature;
                }
                //创建写入文件
                s = NewWritableFile(fs_.get(), manifest, &file, file_options);
                if (!s.ok()) {
                    return s;
                }
                //文件类型set
                FileTypeSet tmp_set = immutable_db_options_.checksum_handoff_file_types;
                //设置预分配块大小
                file->SetPreallocationBlockSize(
                    immutable_db_options_.manifest_preallocation_size);
                //创建写入器
                std::unique_ptr<WritableFileWriter> file_writer(new WritableFileWriter(
                    std::move(file), manifest, file_options, immutable_db_options_.clock,
                    io_tracer_, nullptr /* stats */,
                    Histograms::HISTOGRAM_ENUM_MAX /* hist_type */,
                    immutable_db_options_.listeners, nullptr,
                    tmp_set.Contains(FileType::kDescriptorFile),
                    tmp_set.Contains(FileType::kDescriptorFile)));
                //写入文件创建  日志文件
                latte::rocksdb::LogWriter log(std::move(file_writer), 0, false);
                std::string record;
                //获得记录
                new_db_edit.EncodeTo(&record);
                //记录 写入日志
                s = log.AddRecord(write_options, record);
                if (s.ok()) {
                    //同步清单更新
                    s = SyncManifest(&immutable_db_options_, write_options, log.file());
                }
            }
            if (s.ok()) {
                // 制作指向新清单文件的“当前”文件。
                s = SetCurrentFile(write_options, fs_.get(), dbname_, 1,
                                immutable_db_options_.metadata_write_temperature,
                                directories_.GetDbDir());
                if (new_filenames) {
                    //添加文件名
                    new_filenames->emplace_back(
                        manifest.substr(manifest.find_last_of("/\\") + 1));
                }
            } else {
                //删除清单文件
                fs_->DeleteFile(manifest, IOOptions(), nullptr).PermitUncheckedError();
            }
            return s;
        }

        //恢复数据
        Status RocksDB::Recover(
            //列族描述符数组
            const std::vector<ColumnFamilyDescriptor>& column_families, bool read_only,
            bool error_if_wal_file_exists, bool error_if_data_exists_in_wals,
            bool is_retry, uint64_t* recovered_seq, RecoveryContext* recovery_ctx,
            bool* can_retry)  {
            mutex_.AssertHeld();

            //写入配置
            const WriteOptions write_options(Env::IOActivity::kDBOpen);
            bool tmp_is_new_db = false;
            //上下文获得是否是创建db
            bool& is_new_db = recovery_ctx ? recovery_ctx->is_new_db_ : tmp_is_new_db;
            assert(db_lock_ == nullptr);
            std::vector<std::string> files_in_dbname;
            //非只读
            if (!read_only) {
                //设置目录
                Status s = directories_.SetDirectories(fs_.get(), dbname_,
                                                    immutable_db_options_.wal_dir,
                                                    immutable_db_options_.db_paths);
                if (!s.ok()) {
                    return s;
                }
                //文件锁
                s = env_->LockFile(LockFileName(dbname_), &db_lock_);
                if (!s.ok()) {
                    return s;
                }
                //当前文件
                std::string current_fname = CurrentFileName(dbname_);
                // 数据库目录中任何 MANIFEST 文件的路径。无论哪个文件都可以。
                // 由于尽力恢复会忽略当前文件，因此存在
                // MANIFEST 表示恢复将恢复现有数据库。如果找不到 MANIFEST
                //，则会创建一个新的数据库。
                std::string manifest_path;
                //是否尽力恢复
                if (!immutable_db_options_.best_efforts_recovery) {
                    //判断文件是否存在
                    s = env_->FileExists(current_fname);
                } else {
                    s = Status::NotFound(); //设置状态为未找到文件
                    IOOptions io_opts;
                    io_opts.do_not_recurse = true;
                    //获得目录文件
                    Status io_s = immutable_db_options_.fs->GetChildren(
                        dbname_, io_opts, &files_in_dbname, /*IODebugContext*=*/nullptr);
                    if (!io_s.ok()) {
                        s = io_s;
                        files_in_dbname.clear();
                    }
                    //遍历文件
                    for (const std::string& file : files_in_dbname) {
                        uint64_t number = 0;
                        FileType type = kWalFile;  // initialize
                        //解析文件名 判断是否为描述文件
                        if (ParseFileName(file, &number, &type) && type == kDescriptorFile) {
                            uint64_t bytes; //文件大小
                            s = env_->GetFileSize(DescriptorFileName(dbname_, number), &bytes); 
                            if (s.ok() && bytes != 0) {
                                // 发现非空的 MANIFEST（描述符日志），因此尽最大努力
                                // 恢复不必将数据库视为空。
                                manifest_path = dbname_ + "/" + file;
                                break;
                            }
                        }
                    }
                }
                if (s.IsNotFound()) { //没找到文件
                    //没找到db 需要创建新db
                    if (immutable_db_options_.create_if_missing) {
                        s = NewDB(&files_in_dbname); //创建db
                        is_new_db = true;
                        if (!s.ok()) {
                            return s;
                        }
                    } else {
                        return Status::InvalidArgument(
                            current_fname, "does not exist (create_if_missing is false)"); //无效状态
                    }
                } else if (s.ok()) {
                    if (immutable_db_options_.error_if_exists) { //如果文件存在的话 报错
                        return Status::InvalidArgument(dbname_,
                                                    "exists (error_if_exists is true)");
                    }
                } else {
                    // 读取文件时发生意外错误
                    assert(s.IsIOError());
                    return s;
                }
                // 验证 file_options_ 和文件系统的兼容性
                {
                    //随机存取文件
                    std::unique_ptr<FSRandomAccessFile> idfile;
                    //文件配置
                    FileOptions customized_fs(file_options_);
                    customized_fs.use_direct_reads |=
                        immutable_db_options_.use_direct_io_for_flush_and_compaction; //使用直接读取
                    //获得文件名
                    const std::string& fname =
                        manifest_path.empty() ? current_fname : manifest_path; //如果manifest 文件名是空的话 使用current文件
                    //通过文件名创建随机存取文件
                    s = fs_->NewRandomAccessFile(fname, customized_fs, &idfile, nullptr);
                    if (!s.ok()) { //创建失败
                        std::string error_str = s.ToString();
                        // 检查不支持的直接 I/O 是否是根本原因
                        customized_fs.use_direct_reads = false; //修改非直接读取
                        s = fs_->NewRandomAccessFile(fname, customized_fs, &idfile, nullptr); //修改参数后再次创建
                        if (s.ok()) {
                            return Status::InvalidArgument(
                                "Direct I/O is not supported by the specified DB."); //成功后返回  指定的 DB 不支持直接 I/O。错误
                        } else {
                            return Status::InvalidArgument(
                                "Found options incompatible with filesystem", error_str.c_str()); //其他原因
                        }
                    }
                }
            } else if (immutable_db_options_.best_efforts_recovery) { // 只读 + 尽力恢复
                assert(files_in_dbname.empty());
                IOOptions io_opts;
                io_opts.do_not_recurse = true;
                //获得文件名
                Status s = immutable_db_options_.fs->GetChildren(
                    dbname_, io_opts, &files_in_dbname, /*IODebugContext*=*/nullptr);
                if (s.IsNotFound()) { //文件夹内文件读取错误
                    return Status::InvalidArgument(dbname_,
                                                    "does not exist (open for read only)"); 
                } else if (s.IsIOError()) { //io 错误
                    return s;
                }
                assert(s.ok());
            }
            assert(is_new_db || db_id_.empty());
            Status s;
            bool missing_table_file = false;
            if (!immutable_db_options_.best_efforts_recovery) { //不尽力恢复
                // Status of reading the descriptor file
                Status desc_status;
                //恢复数据
                s = versions_->Recover(column_families, read_only, &db_id_,
                                    /*no_error_if_files_missing=*/false, is_retry,
                                    &desc_status);
                //允许未检查错误
                desc_status.PermitUncheckedError();
                //是否重试
                if (is_retry) {
                    RecordTick(stats_, FILE_READ_CORRUPTION_RETRY_COUNT);
                    if (desc_status.ok()) {
                        RecordTick(stats_, FILE_READ_CORRUPTION_RETRY_SUCCESS_COUNT);
                    }
                }
                if (can_retry) {
                    // 如果我们是第一次打开，失败可能是由于
                    // 损坏的 MANIFEST 文件（可能导致 log::Reader
                    // 检测到损坏的记录，或者由于
                    // 丢弃格式错误的尾部记录而导致 SST 文件未找到错误）
                    if (!is_retry &&
                        (desc_status.IsCorruption() || s.IsNotFound() || s.IsCorruption()) &&
                        CheckFSFeatureSupport(fs_.get(),   
                                                FSSupportedOps::kVerifyAndReconstructRead)) {
                        *can_retry = true;
                        ROCKS_LOG_ERROR(
                            immutable_db_options_.info_log,
                            "Possible corruption detected while replaying MANIFEST %s, %s. "
                            "Will be retried.",
                            desc_status.ToString().c_str(), s.ToString().c_str());
                    } else {
                        *can_retry = false;
                    }
                }
            } else { //尽力恢复
                assert(!files_in_dbname.empty());
                //尝试恢复
                s = versions_->TryRecover(column_families, read_only, files_in_dbname,
                                        &db_id_, &missing_table_file);
                if (s.ok()) {
                    // TryRecover 可能会删除前一个 column_family_set_。
                    column_family_memtables_.reset(
                        new ColumnFamilyMemTablesImpl(versions_->GetColumnFamilySet()));
                }
            }
            if (!s.ok()) {
                return s;
            }
            if (s.ok() && !read_only) {
                for (auto cfd : *versions_->GetColumnFamilySet()) { //遍历column family
                    auto& moptions = *cfd->GetLatestMutableCFOptions(); //获取最新可变CF选项
                    // 当启用 level_compaction_dynamic_level_bytes 时，尝试将文件从最底层开始沿 LSM 树向下移动。这应该仅在用户迁移到启用此选项时才有用。
                    // 如果用户从具有较小级别乘数的级别压缩或通用压缩迁移，则可能存在太多非空级别，并且此处的简单移动不足以进行迁移。需要额外的压缩来消除不必要的级别。
                    // 请注意，此步骤会在不咨询 SSTPartitioner 的情况下将文件向下移动到 LSM。如果用户想要对 SST 文件进行分区，则仍然需要进一步压缩。
                    // 请注意，在此步骤中移动的文件可能不遵守目标级别的压缩选项。
                    if (cfd->ioptions()->compaction_style ==
                            CompactionStyle::kCompactionStyleLevel &&
                        cfd->ioptions()->level_compaction_dynamic_level_bytes &&
                        !moptions.disable_auto_compactions) {
                        int to_level = cfd->ioptions()->num_levels - 1;
                        // 最后一级被保留
                        // allow_ingest_behind 不支持 Level Compaction，
                        // 并且 per_key_placement 可以对 Level 进行无限压缩循环
                        // Compaction。此处调整 to_level 以确保安全。
                        if (cfd->ioptions()->allow_ingest_behind ||
                            moptions.preclude_last_level_data_seconds > 0) {
                            to_level -= 1;
                        }
                        // 此列族是否有可随意移动的级别
                        bool moved = false;
                        // 从 to_level 开始填充 LSM，每次向上填充一级。
                        // 一些循环不变量（当最后一级未保留时）：
                        // - (from_level, to_level] 中的级别为空，并且
                        // - (to_level, last_level] 中的级别非空。
                        for (int from_level = to_level; from_level >= 0; --from_level) {
                            const std::vector<FileMetaData*>& level_files =
                                cfd->current()->storage_info()->LevelFiles(from_level);
                            if (level_files.empty() || from_level == 0) {
                                continue;
                            }
                            assert(from_level <= to_level);
                            // 将文件从 `from_level` 移动到 `to_level`
                            if (from_level < to_level) {
                                if (!moved) {
                                    // 对于具有 7 个级别的 LSM，lsm_state 将看起来像"[1,2,3,4,5,6,0]"
                                    std::string lsm_state = "[";
                                    for (int i = 0; i < cfd->ioptions()->num_levels; ++i) {
                                        lsm_state += std::to_string(
                                            cfd->current()->storage_info()->NumLevelFiles(i)); 
                                        if (i < cfd->ioptions()->num_levels - 1) {
                                            lsm_state += ",";
                                        }
                                    }
                                    lsm_state += "]";
                                    ROCKS_LOG_WARN(immutable_db_options_.info_log,
                                                    "[%s] Trivially move files down the LSM when open "
                                                    "with level_compaction_dynamic_level_bytes=true,"
                                                    " lsm_state: %s (Files are moved only if DB "
                                                    "Recovery is successful).",
                                                    cfd->GetName().c_str(), lsm_state.c_str());
                                    moved = true;
                                }
                                ROCKS_LOG_WARN(
                                    immutable_db_options_.info_log,
                                    "[%s] Moving %zu files from from_level-%d to from_level-%d",
                                    cfd->GetName().c_str(), level_files.size(), from_level,
                                    to_level);
                                VersionEdit edit;
                                edit.SetColumnFamily(cfd->GetID());  //设置column family id
                                for (const FileMetaData* f : level_files) {   //遍历等级
                                    edit.DeleteFile(from_level, f->fd.GetNumber()); //删除文件
                                    edit.AddFile(to_level, f->fd.GetNumber(), f->fd.GetPathId(),
                                                f->fd.GetFileSize(), f->smallest, f->largest,
                                                f->fd.smallest_seqno, f->fd.largest_seqno,
                                                f->marked_for_compaction,
                                                f->temperature,  // this can be different from
                                                // `last_level_temperature`
                                                f->oldest_blob_file_number, f->oldest_ancester_time,
                                                f->file_creation_time, f->epoch_number,
                                                f->file_checksum, f->file_checksum_func_name,
                                                f->unique_id, f->compensated_range_deletion_size,
                                                f->tail_size, f->user_defined_timestamps_persisted);  //添加文件
                                    ROCKS_LOG_WARN(immutable_db_options_.info_log,
                                                    "[%s] Moving #%" PRIu64
                                                    " from from_level-%d to from_level-%d %" PRIu64
                                                    " bytes\n",
                                                    cfd->GetName().c_str(), f->fd.GetNumber(),
                                                    from_level, to_level, f->fd.GetFileSize());
                                }
                                recovery_ctx->UpdateVersionEdits(cfd, edit); //更新
                            }
                            --to_level;
                        }
                    }
                }
            }
            if (is_new_db) {
                // Already set up DB ID in NewDB
            } else if (immutable_db_options_.write_dbid_to_manifest && recovery_ctx) {
                VersionEdit edit;
                s = SetupDBId(write_options, read_only, is_new_db, is_retry, &edit); //设置dbid 
                recovery_ctx->UpdateVersionEdits(
                    versions_->GetColumnFamilySet()->GetDefault(), edit);// 把edit写入到manifest
            } else {
                s = SetupDBId(write_options, read_only, is_new_db, is_retry, nullptr); //只设置dbid
            }
            assert(!s.ok() || !db_id_.empty());
            ROCKS_LOG_INFO(immutable_db_options_.info_log, "DB ID: %s\n", db_id_.c_str());
            if (s.ok() && !read_only) {
                s = MaybeUpdateNextFileNumber(recovery_ctx);  //也许更新下一个文件编号
            }

            if (immutable_db_options_.paranoid_checks && s.ok()) {  //偏执检查
                s = CheckConsistency(); //一致性检查
            }
            if (s.ok() && !read_only) { //非只读
                // TODO：使用上面的 SetDirectories 共享文件描述符（FSDirectory）
                std::map<std::string, std::shared_ptr<FSDirectory>> created_dirs; //文件系统目录
                for (auto cfd : *versions_->GetColumnFamilySet()) { //遍历column family
                    s = cfd->AddDirectories(&created_dirs); //添加目录
                    if (!s.ok()) {
                        return s;
                    }
                }
            }

            std::vector<std::string> files_in_wal_dir;   //wal 目录中的文件
            if (s.ok()) {
                // 恢复 wals 之前的初始 max_total_in_memory_state_。日志恢复
                // 可能会检查此值来决定是否刷新。
                max_total_in_memory_state_ = 0;  //计算内存大小
                for (auto cfd : *versions_->GetColumnFamilySet()) { //遍历column family
                    auto* mutable_cf_options = cfd->GetLatestMutableCFOptions();  //获取最新可变CF选项
                    max_total_in_memory_state_ += mutable_cf_options->write_buffer_size *         //写入缓冲区大小
                                                    mutable_cf_options->max_write_buffer_number;  //最大写入缓冲区数量
                }

                SequenceNumber next_sequence(kMaxSequenceNumber);  //序列号
                default_cf_handle_ = new ColumnFamilyHandleImpl(
                    versions_->GetColumnFamilySet()->GetDefault(), this, &mutex_); //创建handle
                default_cf_internal_stats_ = default_cf_handle_->cfd()->internal_stats(); //内部统计状态

                // 从所有比描述符中命名的日志文件更新的日志文件中恢复
                //（前一个版本可能添加了新的日志文件，但没有在描述符中注册它们）。
                //
                // 请注意，prev_log_number() 不再使用，但我们要注意它，
                // 以防我们恢复由旧版本的 rocksdb 生成的数据库。
                auto wal_dir = immutable_db_options_.GetWalDir();  //获得wal目录
                if (!immutable_db_options_.best_efforts_recovery) { //非尽力恢复
                    IOOptions io_opts;
                    io_opts.do_not_recurse = true;
                    s = immutable_db_options_.fs->GetChildren(
                        wal_dir, io_opts, &files_in_wal_dir, /*IODebugContext*=*/nullptr); //读取wal文件夹下的文件
                }
                if (s.IsNotFound()) { //读取失败
                    return Status::InvalidArgument("wal_dir not found", wal_dir);
                } else if (!s.ok()) {
                    return s;
                }

                std::unordered_map<uint64_t, std::string> wal_files;
                for (const auto& file : files_in_wal_dir) { //遍历wal文件夹下的文件
                    uint64_t number;
                    FileType type; //文件类型
                    if (ParseFileName(file, &number, &type) && type == kWalFile) { //是否wal文件
                        if (is_new_db) { //是否是新文件
                            return Status::Corruption(
                                "While creating a new Db, wal_dir contains "
                                "existing log file: ",
                                file);
                        } else {
                            wal_files[number] = LogFileName(wal_dir, number); //创建文件
                        }
                    }
                }

                if (immutable_db_options_.track_and_verify_wals_in_manifest) {  //清单中的跟踪和验证
                    if (!immutable_db_options_.best_efforts_recovery) {  //非尽力恢复
                        // Verify WALs in MANIFEST.
                        s = versions_->GetWalSet().CheckWals(env_, wal_files);  //检查wal

                    }  // else since best effort recovery does not recover from WALs, no need
                        // to check WALs.
                } else if (!versions_->GetWalSet().GetWals().empty()) { //获得wals 非空
                    // 跟踪已禁用，请从 MANIFEST 中清除先前跟踪的 WAL，
                    // 否则，将来，如果再次启用 WAL 跟踪，
                    // 由于禁用 WAL 跟踪时删除的 WAL 不会保留
                    // 到 MANIFEST 中，因此 WAL 检查可能会失败。
                    VersionEdit edit;
                    WalNumber max_wal_number =
                        versions_->GetWalSet().GetWals().rbegin()->first; //获得wal序列号
                    edit.DeleteWalsBefore(max_wal_number + 1); //删除wal版本之前
                    assert(recovery_ctx != nullptr);
                    assert(versions_->GetColumnFamilySet() != nullptr); 
                    recovery_ctx->UpdateVersionEdits(
                        versions_->GetColumnFamilySet()->GetDefault(), edit); //设置edit
                }
                if (!s.ok()) {
                    return s;
                }

                if (!wal_files.empty()) { //wal文件为非空  
                    if (error_if_wal_file_exists) {  //错误信息：如果 wal_file 存在
                        return Status::Corruption(
                            "The db was opened in readonly mode with error_if_wal_file_exists"
                            "flag but a WAL file already exists");
                    } else if (error_if_data_exists_in_wals) { //如果数据在 Wals 中存在，则出错
                        for (auto& wal_file : wal_files) { //遍历wal 文件
                            uint64_t bytes;
                            s = env_->GetFileSize(wal_file.second, &bytes); //获得文件大小
                            if (s.ok()) {
                                if (bytes > 0) {
                                    return Status::Corruption(
                                        "error_if_data_exists_in_wals is set but there are data "
                                        " in WAL files.");
                                }
                            }
                        }
                    }
                }

                if (!wal_files.empty()) { //wal 非空
                    // 按照墙的生成顺序恢复
                    std::vector<uint64_t> wals;
                    wals.reserve(wal_files.size()); //设置长度
                    for (const auto& wal_file : wal_files) { //遍历wal 文件
                        wals.push_back(wal_file.first); 
                    }
                    std::sort(wals.begin(), wals.end()); //排序

                    bool corrupted_wal_found = false; //损坏的wal
                    s = RecoverLogFiles(wals, &next_sequence, read_only, is_retry,
                                        &corrupted_wal_found, recovery_ctx); //恢复日志文件
                    if (corrupted_wal_found && recovered_seq != nullptr) { //恢复序列
                        *recovered_seq = next_sequence; //下个序列号
                    }
                    if (!s.ok()) {
                        // 如果恢复失败，清除内存表
                        for (auto cfd : *versions_->GetColumnFamilySet()) { //遍历column family
                            cfd->CreateNewMemtable(*cfd->GetLatestMutableCFOptions(),
                                                    kMaxSequenceNumber); //创建新的内存表
                        }
                    }
                }
            }

            if (read_only) { //只读
                // 如果我们以只读方式打开，则需要更新 options_file_number_
                // 以反映最新的 OPTIONS 文件。这对于常规
                // 读写数据库实例无关紧要，因为 options_file_number_ 稍后将
                // 更新为 RenameTempFileToOptionsFile 中的versions_->NewFileNumber()。
                std::vector<std::string> filenames;
                if (s.ok()) {
                    const std::string normalized_dbname = NormalizePath(dbname_);  //规范化db路径
                    const std::string normalized_wal_dir =
                        NormalizePath(immutable_db_options_.GetWalDir()); //规范化wal路径
                    if (immutable_db_options_.best_efforts_recovery) { //尽力恢复
                        filenames = std::move(files_in_dbname);  //db文件夹下的文件
                    } else if (normalized_dbname == normalized_wal_dir) { //db 路径和wal 路径相等
                        filenames = std::move(files_in_wal_dir); //wal文件夹下的文件
                    } else {
                        IOOptions io_opts;
                        io_opts.do_not_recurse = true; //非递归
                        s = immutable_db_options_.fs->GetChildren(
                            GetName(), io_opts, &filenames, /*IODebugContext*=*/nullptr); //读取 name_ 的文件夹下
                    }
                }
                if (s.ok()) {
                    uint64_t number = 0;
                    uint64_t options_file_number = 0;
                    FileType type;
                    for (const auto& fname : filenames) {  //遍历文件
                        if (ParseFileName(fname, &number, &type) && type == kOptionsFile) { //判断是否koptions文件
                                options_file_number = std::max(number, options_file_number); //获得文件序列号
                        }
                    }
                    versions_->options_file_number_ = options_file_number; //设置options 文件序列号
                    uint64_t options_file_size = 0;
                    if (options_file_number > 0) {
                        s = env_->GetFileSize(OptionsFileName(GetName(), options_file_number),
                                            &options_file_size); //文件大小
                    }
                    versions_->options_file_size_ = options_file_size;
                }
            }
            return s;
        }

        //打开rocksdb
        Status RocksDB::Open(const DBOptions& db_options, const std::string& dbname,
                    const std::vector<ColumnFamilyDescriptor>& column_families,
                    std::vector<ColumnFamilyHandle*>* handles, DB** dbptr,
                    const bool seq_per_batch, const bool batch_per_txn,
                    const bool is_retry, bool* can_retry) {
            const WriteOptions write_options(Env::IOActivity::kDBOpen);
            const ReadOptions read_options(Env::IOActivity::kDBOpen);
            //验证table 的options 正确性
            Status s = ValidateOptionsByTable(db_options, column_families);
            if (!s.ok()) {
                return s;
            }
            s = ValidateOptions(db_options, column_families);
            if (!s.ok()) {
                return s;
            }

            *dbptr = nullptr;
            assert(handles);
            handles->clear(); //打开前 先清空一下

            size_t max_write_buffer_size = 0;  //最大写入buffer大小 多个列族中取最大
            for (const auto& cf : column_families) {
                max_write_buffer_size =
                    std::max(max_write_buffer_size, cf.options.write_buffer_size);
            }

            //创建的rocksdb 对象    参数  ，db名字 ，批量序号 
            RocksDB* impl = new RocksDB(db_options, dbname, seq_per_batch, batch_per_txn); 
            //info_log 创建是否成功，  如果失败的话  返回状态为创建info_log失败的原因
            if (!impl->immutable_db_options_.info_log) {
                s = impl->init_logger_creation_s_;
                delete impl;
                return s;
            } else {
                assert(impl->init_logger_creation_s_.ok());
            }
            //创建wal文件夹
            s = impl->env_->CreateDirIfMissing(impl->immutable_db_options_.GetWalDir());
            if (s.ok()) { //创建成功
                std::vector<std::string> paths; //收集地址
                //db的地址
                for (auto& db_path : impl->immutable_db_options_.db_paths) {
                    paths.emplace_back(db_path.path);
                }
                //收集cf的地址
                for (auto& cf : column_families) {
                    for (auto& cf_path : cf.options.cf_paths) {
                        paths.emplace_back(cf_path.path);
                    }
                }
                //通过收集到的文件夹，创建文件夹
                for (const auto& path : paths) {
                    s = impl->env_->CreateDirIfMissing(path);
                    if (!s.ok()) {
                        break;
                    }
                }

                // 为了从 NoSpace() 错误中恢复，我们只能处理
                // 数据库存储在单个路径中的情况
                if (paths.size() <= 1) {
                    //启动自动恢复
                    impl->error_handler_.EnableAutoRecovery();
                }
            }
            if (s.ok()) {
                //创建档案目录
                s = impl->CreateArchivalDirectory();
            }

            if (!s.ok()) { //之前的状态不是ok 从这里返回
                delete impl;
                return s;
            }
            //Wal 文件夹是否与 DB 路径相同
            impl->wal_in_db_path_ = impl->immutable_db_options_.IsWalDirSameAsDBPath();
            RecoveryContext recovery_ctx; //恢复上下文对象
            //加锁
            impl->options_mutex_.Lock();
            impl->mutex_.Lock();

            // 处理 create_if_missing、error_if_exists
            uint64_t recovered_seq(kMaxSequenceNumber);
            //恢复数据
            s = impl->Recover(column_families, false /* read_only */,
                                false /* error_if_wal_file_exists */,
                                false /* error_if_data_exists_in_wals */, is_retry,
                                &recovered_seq, &recovery_ctx, can_retry);
            if (s.ok()) { //恢复成功
                uint64_t new_log_number = impl->versions_->NewFileNumber(); //获得log_number
                latte::rocksdb::LogWriter* new_log = nullptr;  //新日志写入对象
                const size_t preallocate_block_size =
                    impl->GetWalPreallocateBlockSize(max_write_buffer_size);  //获取 Wal 预分配块大小
                s = impl->CreateWAL(write_options, new_log_number, 0 /*recycle_log_number*/,
                                    preallocate_block_size, &new_log);  //创建wal
                if (s.ok()) {
                    // 防止回收前一个实例创建的日志文件。
                    // 它们可能在 alive_log_file_ 中，否则可能会被回收。
                    impl->min_log_number_to_recycle_ = new_log_number;   //回收的最小日志数
                }

                if (s.ok()) {
                    InstrumentedMutexLock wl(&impl->log_write_mutex_);  //检测互斥锁
                    impl->logfile_number_ = new_log_number; //设置当前日志数
                    assert(new_log != nullptr); 
                    assert(impl->logs_.empty());
                    impl->logs_.emplace_back(new_log_number, new_log); //添加日志对象
                }

                if (s.ok()) {
                    //添加活动的日志文件
                    impl->alive_log_files_.emplace_back(impl->logfile_number_); //尾部添加对象
                    // 在 WritePrepared 中，序列号可能会有间隙。
                    // 这会破坏我们在 kPointInTimeRecovery 中使用的技巧，
                    // 该技巧假设损坏日志之后的第一个日志序列比我们从 wals 读取的最后一个序列大一个。
                    // 为了让这个技巧继续发挥作用，我们在恢复后的第一个日志中添加了一个具有预期序列的虚拟条目。
                    // 在非 WritePrepared 情况下，恢复后的新日志也可能为空，
                    // 因此缺少连续的序列提示以区分中间日志损坏和恢复后剩余的损坏日志。
                    // 这种情况也将通过虚拟写入来解决。
                    if (recovered_seq != kMaxSequenceNumber) {  // 恢复返回
                        WriteBatch empty_batch;
                        WriteBatchInternal::SetSequence(&empty_batch, recovered_seq); //写入批处理内部
                        uint64_t log_used, log_size;
                        rocksdb::LogWriter* log_writer = impl->logs_.back().writer; //取出写入对象
                        LogFileNumberSize& log_file_number_size = impl->alive_log_files_.back(); //返回最后一个logfilenumbersize

                        assert(log_writer->get_log_number() == log_file_number_size.number); //判断写入logwrite和logfilenumbersize对象配对
                        impl->mutex_.AssertHeld();
                        s = impl->WriteToWAL(empty_batch, write_options, log_writer, &log_used,
                                            &log_size, log_file_number_size); //写入wal
                        if (s.ok()) {
                            // 需要 fsync，否则电源重置后可能会丢失。
                            s = impl->FlushWAL(write_options, false);
                            // TEST_SYNC_POINT_CALLBACK("DBImpl::Open::BeforeSyncWAL", /*arg=*/&s);
                            IOOptions opts;
                            if (s.ok()) {
                                s = WritableFileWriter::PrepareIOOptions(write_options, opts);
                            }
                            if (s.ok()) {
                                s = log_writer->file()->Sync(opts,
                                                            impl->immutable_db_options_.use_fsync);
                            }
                        }
                    }
                }
            }

            if (s.ok()) {
                s = impl->LogAndApplyForRecovery(recovery_ctx); //记录并申请恢复
            }

            if (s.ok() && !impl->immutable_db_options_.write_identity_file) { //如果不需要表示文件的话删除
                // 成功恢复后，删除过时的 IDENTITY 文件以避免 DB ID
                // 不一致
                impl->env_->DeleteFile(IdentityFileName(impl->dbname_))
                    .PermitUncheckedError();
            }

            if (s.ok() && impl->immutable_db_options_.persist_stats_to_disk) { //状态记录功能开启的话
                impl->mutex_.AssertHeld();
                s = impl->InitPersistStatsColumnFamily();  //初始化状态记录
            }

            if (s.ok()) {
                // set column family handles
                for (const auto& cf : column_families) { //遍历columnfamily描述符
                    auto cfd =
                        impl->versions_->GetColumnFamilySet()->GetColumnFamily(cf.name);  //通过名字获得ColumnFamilyData
                    if (cfd != nullptr) { //找到column family 具体数据
                        handles->push_back(
                            new ColumnFamilyHandleImpl(cfd, impl, &impl->mutex_));  //创建handle 并添加
                        impl->NewThreadStatusCfInfo(cfd); //设置新线程状态cf
                    } else {
                        if (db_options.create_missing_column_families) { //如果配置是可创建的话 就创建handle
                            // missing column family, create it
                            ColumnFamilyHandle* handle = nullptr;
                            impl->mutex_.Unlock(); //解锁
                            // 注意：通常在 WrapUpCreateColumnFamilies 中完成的工作将
                            // 在下面单独完成。
                            s = impl->CreateColumnFamilyImpl(read_options, write_options,
                                                            cf.options, cf.name, &handle); //创建columnfamily对象 以及对应的handle
                            impl->mutex_.Lock(); //加锁
                            if (s.ok()) {
                                handles->push_back(handle); //添加handle
                            } else {
                                break;
                            }
                        } else {
                            s = Status::InvalidArgument("Column family not found", cf.name); //不支持创建 就返回没找到column family
                            break;
                        }
                    }
                }
            }


            if (s.ok()) { 
                SuperVersionContext sv_context(/* create_superversion */ true);
                for (auto cfd : *impl->versions_->GetColumnFamilySet()) { //遍历所有的columnfamily
                    impl->InstallSuperVersionAndScheduleWork(
                        cfd, &sv_context, *cfd->GetLatestMutableCFOptions()); //安装超级版本和计划工作 和 获取最新可变CF选项                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
                }
                sv_context.Clean();
            }

            if (s.ok() && impl->immutable_db_options_.persist_stats_to_disk) {  //记录状态
                // try to read format version
                // 尝试去读取格式化版本
                s = impl->PersistentStatsProcessFormatVersion();
            }

            if (s.ok()) {
                for (auto cfd : *impl->versions_->GetColumnFamilySet()) { //遍历column family
                    if (!cfd->mem()->IsSnapshotSupported()) {  //不支持快照
                        impl->is_snapshot_supported_ = false;
                    }
                    if (cfd->ioptions()->merge_operator != nullptr &&
                        !cfd->mem()->IsMergeOperatorSupported()) {  //有合并对象但是配置不支持合并   返回错误
                        s = Status::InvalidArgument(
                            "The memtable of column family %s does not support merge operator "
                            "its options.merge_operator is non-null",
                            cfd->GetName().c_str()); //无效参数
                    }
                    if (!s.ok()) {
                        break;
                    }
                }
            }
            // TEST_SYNC_POINT("DBImpl::Open:Opened");
            Status persist_options_status; //持久选项状态
            if (s.ok()) {
                // 在安排压缩之前保留 RocksDB 选项。
                // WriteOptionsFile() 将在内部释放并锁定互斥锁。
                persist_options_status =
                    impl->WriteOptionsFile(write_options, true /*db_mutex_already_held*/);
                *dbptr = impl;  //返回 rocksdb对象
                impl->opened_successfully_ = true; //已成功打开
            } else {
                persist_options_status.PermitUncheckedError();
            }
            impl->mutex_.Unlock();  //解锁

            auto sfm = static_cast<SstFileManagerImpl*>(
                impl->immutable_db_options_.sst_file_manager.get());  //sst文件管理器
            if (s.ok() && sfm) {
                // 为 SstFileManager 设置统计信息 ptr，以转储 DeleteScheduler 的统计信息。
                sfm->SetStatisticsPtr(impl->immutable_db_options_.statistics); //设置统计对象指针
                ROCKS_LOG_INFO(impl->immutable_db_options_.info_log,
                            "SstFileManager instance %p", sfm);

                impl->TrackExistingDataFiles(recovery_ctx.existing_data_files_); // 追踪现有数据文件

                // 保留一些磁盘缓冲区空间。这是一种启发式方法 - 当我们用完磁盘空间时，
                // 这可确保在恢复 DB 写入之前至少有 write_buffer_size 大小的可用空间。在磁盘空间不足的情况下，我们希望避免由于频繁的 WAL 写入失败和由此导致的强制刷新而产生大量小的 L0 文件
                sfm->ReserveDiskBuffer(max_write_buffer_size,
                                    impl->immutable_db_options_.db_paths[0].path); //预留磁盘缓冲区
            }

            if (s.ok()) {
                // 当数据库停止时，可能有一些 .trash 文件
                // 尚未删除，当我们打开数据库时，我们会找到这些 .trash
                // 文件并安排删除它们（如果未使用 SstFileManager，则立即删除）。
                // 请注意，我们只在调用
                // `TrackExistingDataFiles` 后才开始执行此操作并删除过时的文件，否则 `max_trash_db_ratio`
                // 无效，并且这些文件的删除不会受到速率限制
                // 这可能会导致丢弃停滞。
                for (const auto& path : impl->CollectAllDBPaths()) {   //收集所有数据库路径
                    DeleteScheduler::CleanupDirectory(impl->immutable_db_options_.env, sfm,
                                                        path)
                        .PermitUncheckedError(); //清理目录
                }
                impl->mutex_.Lock(); //加锁
                // 这将进行一次全面扫描。
                impl->DeleteObsoleteFiles();  //删除过时文件
                // TEST_SYNC_POINT("DBImpl::Open:AfterDeleteFiles");
                impl->MaybeScheduleFlushOrCompaction(); //可能安排刷新或压缩
                impl->mutex_.Unlock(); //解锁
            }

            if (s.ok()) {
                ROCKS_LOG_HEADER(impl->immutable_db_options_.info_log, "DB pointer %p",
                                impl);
                LogFlush(impl->immutable_db_options_.info_log); //日志刷新
                if (!impl->WALBufferIsEmpty()) { // WAL 缓冲区为非空
                    s = impl->FlushWAL(write_options, false); //刷出 wal
                    if (s.ok()) {
                        // 需要同步，否则 WAL 缓冲数据可能会在电源重置后丢失。
                        rocksdb::LogWriter* log_writer = impl->logs_.back().writer;
                        IOOptions opts;
                        s = WritableFileWriter::PrepareIOOptions(write_options, opts); //准备选项
                        if (s.ok()) {
                            s = log_writer->file()->Sync(opts,
                                                        impl->immutable_db_options_.use_fsync);
                        }
                    }
                }
                if (s.ok() && !persist_options_status.ok()) { //保留选项状态为成功的时候 返回错误
                    s = Status::IOError(
                        "DB::Open() failed --- Unable to persist Options file",
                        persist_options_status.ToString());
                }
            }
            if (!s.ok()) {
                ROCKS_LOG_WARN(impl->immutable_db_options_.info_log,
                            "DB::Open() failed: %s", s.ToString().c_str());
            }
            if (s.ok()) {
                s = impl->StartPeriodicTaskScheduler(); // 启动周期性任务调度器
            }
            if (s.ok()) {
                s = impl->RegisterRecordSeqnoTimeWorker(read_options, write_options,
                                                        recovery_ctx.is_new_db_); //注册记录序号时间工作者
            }
            impl->options_mutex_.Unlock();  //解锁
            if (!s.ok()) {
                for (auto* h : *handles) { //失败的话 清空handle
                    delete h;
                }
                handles->clear(); //清空 handles
                delete impl;  //删除rocksdb对象                *dbptr = nullptr;
            }
            return s;
        }




        
        //打开rocksdb
        Status RocksDB::Open(const OpenOptions& db_options, 
            const std::string& dbname, 
            const std::vector<ColumnFamilyDescriptor>& column_families,
            std::vector<ColumnFamilyHandle*>* handles,
            DB** dbptr) {
            const bool kSeqPerBatch = true;
            const bool kBatchPerTxn = true;
            ThreadStatusUtil::SetEnableTracking(db_options.enable_thread_tracking); //线程设置启动跟踪
            ThreadStatusUtil::SetThreadOperation(ThreadStatus::OperationType::OP_DBOPEN); //设置线程操作 （dbopen）
            bool can_retry = false; //是否重试  
            Status s;
            do {
                s = Open(db_options, dbname, column_families, handles, dbptr,
                     !kSeqPerBatch, kBatchPerTxn, can_retry, &can_retry);
            } while(!s.ok() && can_retry);
            ThreadStatusUtil::ResetThreadStatus(); //重新设置线程状态
            return s;
        };

        Status RocksDB::Open(const RocksdbOpenOptions& options, const std::string& dbname, DB** dbptr) {
            OpenOptions db_options(options); //重新设置openOptions
            ColumnFamilyOptions cf_options(options); //列族选项
            std::vector<ColumnFamilyDescriptor> column_families; //列族描述数组
            column_families.emplace_back(kDefaultColumnFamilyName, cf_options);  //默认 default
            if (db_options.persist_stats_to_disk) { // 将统计数据保存到磁盘  //记录到 ___rocksdb_stats_history___ 列族里
                column_families.emplace_back(kPersistentStatsColumnFamilyName, cf_options);
            }
            std::vector<ColumnFamilyHandle*> handles; //创建出来的列族句柄数组
            Status s = Open(db_options, dbname, column_families, &handles, dbptr); //打开
            if (s.ok()) { //打开成功
                if (db_options.persist_stats_to_disk) { //判断列族句柄数组个数是否正确
                    assert(handles.size() == 2);
                } else {
                    assert(handles.size() == 1);
                }
                // 我可以删除句柄，因为 DBImpl 始终持有对
                // 默认列系列的引用
                if (db_options.persist_stats_to_disk && handles[1] != nullptr) { //关闭___rocksdb_stats_history___
                    delete handles[1];
                }
                delete handles[0];  //关闭default
            }
            return s;
        }


        OpenOptions SanitizeOptions(const std::string& dbname, const OpenOptions& src,
                        bool read_only, Status* logger_creation_s) {
            auto db_options =
                SanitizeOptions(dbname, DBOptions(src), read_only, logger_creation_s); //DBOptions
            ImmutableDBOptions immutable_db_options(db_options);
            auto cf_options =
                SanitizeOptions(immutable_db_options, ColumnFamilyOptions(src)); //ColumnFamilyOptions 方法在column_family_options.h 
            return OpenOptions(db_options, cf_options);
        }


        // DBOptions SanitizeOptions(const std::string& dbname, const DBOptions& src,
        //                   bool read_only, Status* logger_creation_s) {
        //     DBOptions result(src);

        //     if (result.env == nullptr) {
        //         result.env = Env::Default();
        //     }

        //     // result.max_open_files means an "infinite" open files.
        //     if (result.max_open_files != -1) {
        //         int max_max_open_files = port::GetMaxOpenFiles();
        //         if (max_max_open_files == -1) {
        //         max_max_open_files = 0x400000;
        //         }
        //         ClipToRange(&result.max_open_files, 20, max_max_open_files);
        //         TEST_SYNC_POINT_CALLBACK("SanitizeOptions::AfterChangeMaxOpenFiles",
        //                                 &result.max_open_files);
        //     }

        //     if (result.info_log == nullptr && !read_only) {
        //         Status s = CreateLoggerFromOptions(dbname, result, &result.info_log);
        //         if (!s.ok()) {
        //              // No place suitable for logging
        //              result.info_log = nullptr;
        //              if (logger_creation_s) {
        //                  *logger_creation_s = s;
        //              }
        //         }
        //     }

        //     if (!result.write_buffer_manager) {
        //         result.write_buffer_manager.reset(
        //             new WriteBufferManager(result.db_write_buffer_size));
        //     }
        //     auto bg_job_limits = DBImpl::GetBGJobLimits(
        //         result.max_background_flushes, result.max_background_compactions,
        //         result.max_background_jobs, true /* parallelize_compactions */);
        //     result.env->IncBackgroundThreadsIfNeeded(bg_job_limits.max_compactions,
        //                                             Env::Priority::LOW);
        //     result.env->IncBackgroundThreadsIfNeeded(bg_job_limits.max_flushes,
        //                                             Env::Priority::HIGH);

        //     if (result.rate_limiter.get() != nullptr) {
        //         if (result.bytes_per_sync == 0) {
        //         result.bytes_per_sync = 1024 * 1024;
        //         }
        //     }

        //     if (result.delayed_write_rate == 0) {
        //         if (result.rate_limiter.get() != nullptr) {
        //         result.delayed_write_rate = result.rate_limiter->GetBytesPerSecond();
        //         }
        //         if (result.delayed_write_rate == 0) {
        //         result.delayed_write_rate = 16 * 1024 * 1024;
        //         }
        //     }

        //     if (result.WAL_ttl_seconds > 0 || result.WAL_size_limit_MB > 0) {
        //         result.recycle_log_file_num = false;
        //     }

        //     if (result.recycle_log_file_num &&
        //         (result.wal_recovery_mode ==
        //             WALRecoveryMode::kTolerateCorruptedTailRecords ||
        //         result.wal_recovery_mode == WALRecoveryMode::kAbsoluteConsistency)) {
        //         // - kTolerateCorruptedTailRecords is inconsistent with recycle log file
        //         //   feature. WAL recycling expects recovery success upon encountering a
        //         //   corrupt record at the point where new data ends and recycled data
        //         //   remains at the tail. However, `kTolerateCorruptedTailRecords` must fail
        //         //   upon encountering any such corrupt record, as it cannot differentiate
        //         //   between this and a real corruption, which would cause committed updates
        //         //   to be truncated -- a violation of the recovery guarantee.
        //         // - kPointInTimeRecovery and kAbsoluteConsistency are incompatible with
        //         //   recycle log file feature temporarily due to a bug found introducing a
        //         //   hole in the recovered data
        //         //   (https://github.com/facebook/rocksdb/pull/7252#issuecomment-673766236).
        //         //   Besides this bug, we believe the features are fundamentally compatible.
        //         result.recycle_log_file_num = 0;
        //     }

        //     if (result.db_paths.size() == 0) {
        //         result.db_paths.emplace_back(dbname, std::numeric_limits<uint64_t>::max());
        //     } else if (result.wal_dir.empty()) {
        //         // Use dbname as default
        //         result.wal_dir = dbname;
        //     }
        //     if (!result.wal_dir.empty()) {
        //         // If there is a wal_dir already set, check to see if the wal_dir is the
        //         // same as the dbname AND the same as the db_path[0] (which must exist from
        //         // a few lines ago). If the wal_dir matches both of these values, then clear
        //         // the wal_dir value, which will make wal_dir == dbname.  Most likely this
        //         // condition was the result of reading an old options file where we forced
        //         // wal_dir to be set (to dbname).
        //         auto npath = NormalizePath(dbname + "/");
        //         if (npath == NormalizePath(result.wal_dir + "/") &&
        //             npath == NormalizePath(result.db_paths[0].path + "/")) {
        //         result.wal_dir.clear();
        //         }
        //     }

        //     if (!result.wal_dir.empty() && result.wal_dir.back() == '/') {
        //         result.wal_dir = result.wal_dir.substr(0, result.wal_dir.size() - 1);
        //     }

        //     // Force flush on DB open if 2PC is enabled, since with 2PC we have no
        //     // guarantee that consecutive log files have consecutive sequence id, which
        //     // make recovery complicated.
        //     if (result.allow_2pc) {
        //         result.avoid_flush_during_recovery = false;
        //     }

        //     ImmutableDBOptions immutable_db_options(result);
        //     if (!immutable_db_options.IsWalDirSameAsDBPath()) {
        //         // Either the WAL dir and db_paths[0]/db_name are not the same, or we
        //         // cannot tell for sure. In either case, assume they're different and
        //         // explicitly cleanup the trash log files (bypass DeleteScheduler)
        //         // Do this first so even if we end up calling
        //         // DeleteScheduler::CleanupDirectory on the same dir later, it will be
        //         // safe
        //         std::vector<std::string> filenames;
        //         IOOptions io_opts;
        //         io_opts.do_not_recurse = true;
        //         auto wal_dir = immutable_db_options.GetWalDir();
        //         Status s = immutable_db_options.fs->GetChildren(
        //             wal_dir, io_opts, &filenames, /*IODebugContext*=*/nullptr);
        //         s.PermitUncheckedError();  //**TODO: What to do on error?
        //         for (std::string& filename : filenames) {
        //         if (filename.find(".log.trash", filename.length() -
        //                                             std::string(".log.trash").length()) !=
        //             std::string::npos) {
        //             std::string trash_file = wal_dir + "/" + filename;
        //             result.env->DeleteFile(trash_file).PermitUncheckedError();
        //         }
        //         }
        //     }

        //     // Create a default SstFileManager for purposes of tracking compaction size
        //     // and facilitating recovery from out of space errors.
        //     if (result.sst_file_manager.get() == nullptr) {
        //         std::shared_ptr<SstFileManager> sst_file_manager(
        //             NewSstFileManager(result.env, result.info_log));
        //         result.sst_file_manager = sst_file_manager;
        //     }

        //     // Supported wal compression types
        //     if (!StreamingCompressionTypeSupported(result.wal_compression)) {
        //         result.wal_compression = kNoCompression;
        //         ROCKS_LOG_WARN(result.info_log,
        //                     "wal_compression is disabled since only zstd is supported");
        //     }

        //     if (!result.paranoid_checks) {
        //         result.skip_checking_sst_file_sizes_on_db_open = true;
        //         ROCKS_LOG_INFO(result.info_log,
        //                     "file size check will be skipped during open.");
        //     }

        //     return result;
        // }


    //     RocksDB::RocksDB(const DBOptions& options, const std::string& dbname,
    //            const bool seq_per_batch, const bool batch_per_txn,
    //            bool read_only)
    //             : dbname_(dbname),
    //                 own_info_log_(options.info_log == nullptr), // 标记以检查我们是否分配并拥有信息日志文件
    //                 init_logger_creation_s_(),  // 初始化日志记录器创建状态
    //                 initial_db_options_(SanitizeOptions(dbname, options, read_only,
    //                                                     &init_logger_creation_s_)), //创建初始数据库选项  如果失败
    //                 env_(initial_db_options_.env),
    //                 io_tracer_(std::make_shared<IOTracer>()),
    //                 immutable_db_options_(initial_db_options_),
    //                 fs_(immutable_db_options_.fs, io_tracer_),
    //                 mutable_db_options_(initial_db_options_),
    //                 stats_(immutable_db_options_.stats),
    //             #ifdef COERCE_CONTEXT_SWITCH
    //                 mutex_(stats_, immutable_db_options_.clock, DB_MUTEX_WAIT_MICROS, &bg_cv_,
    //                         immutable_db_options_.use_adaptive_mutex),
    //             #else   // COERCE_CONTEXT_SWITCH
    //                 mutex_(stats_, immutable_db_options_.clock, DB_MUTEX_WAIT_MICROS,
    //                         immutable_db_options_.use_adaptive_mutex),
    //             #endif  // COERCE_CONTEXT_SWITCH
    //                 default_cf_handle_(nullptr),
    //                 error_handler_(this, immutable_db_options_, &mutex_),
    //                 event_logger_(immutable_db_options_.info_log.get()),
    //                 max_total_in_memory_state_(0),
    //                 file_options_(BuildDBOptions(immutable_db_options_, mutable_db_options_)),
    //                 file_options_for_compaction_(fs_->OptimizeForCompactionTableWrite(
    //                     file_options_, immutable_db_options_)),
    //                 seq_per_batch_(seq_per_batch),
    //                 batch_per_txn_(batch_per_txn),
    //                 next_job_id_(1),
    //                 shutting_down_(false),
    //                 reject_new_background_jobs_(false),
    //                 db_lock_(nullptr),
    //                 manual_compaction_paused_(false),
    //                 bg_cv_(&mutex_),
    //                 logfile_number_(0),
    //                 log_dir_synced_(false),
    //                 log_empty_(true),
    //                 persist_stats_cf_handle_(nullptr),
    //                 log_sync_cv_(&log_write_mutex_),
    //                 total_log_size_(0),
    //                 is_snapshot_supported_(true),
    //                 write_buffer_manager_(immutable_db_options_.write_buffer_manager.get()),
    //                 write_thread_(immutable_db_options_),
    //                 nonmem_write_thread_(immutable_db_options_),
    //                 write_controller_(mutable_db_options_.delayed_write_rate),
    //                 last_batch_group_size_(0),
    //                 unscheduled_flushes_(0),
    //                 unscheduled_compactions_(0),
    //                 bg_bottom_compaction_scheduled_(0),
    //                 bg_compaction_scheduled_(0),
    //                 num_running_compactions_(0),
    //                 bg_flush_scheduled_(0),
    //                 num_running_flushes_(0),
    //                 bg_purge_scheduled_(0),
    //                 disable_delete_obsolete_files_(0),
    //                 pending_purge_obsolete_files_(0),
    //                 delete_obsolete_files_last_run_(immutable_db_options_.clock->NowMicros()),
    //                 has_unpersisted_data_(false),
    //                 unable_to_release_oldest_log_(false),
    //                 num_running_ingest_file_(0),
    //                 wal_manager_(immutable_db_options_, file_options_, io_tracer_,
    //                             seq_per_batch),
    //                 bg_work_paused_(0),
    //                 bg_compaction_paused_(0),
    //                 refitting_level_(false),
    //                 opened_successfully_(false),
    //                 periodic_task_scheduler_(),
    //                 two_write_queues_(options.two_write_queues),
    //                 manual_wal_flush_(options.manual_wal_flush),
    //                 // last_sequencee_ 始终由主队列维护，主队列还会将数据写入
    //                 // memtable。禁用 two_write_queues_ 时，memtable 中的最后一个序列与发布给读者的最后一个序列相同。启用它但禁用 seq_per_batch_ 时，memtable 中的最后一个序列仍
    //                 // 表示最后发布的序列，因为进入第二个队列的 wal-only 写入不会消耗序列号。否则，第二个队列执行的写入可能会更改读者可见的内容。在这种情况下，last_seq_same_as_publish_seq_==false，第二个队列维护一个
    //                 // 单独的变量来指示最后发布的序列。
    //                 last_seq_same_as_publish_seq_(
    //                     !(seq_per_batch && options.two_write_queues)),
    //                 // 由于 seq_per_batch_ 目前仅由 WritePreparedTxn 设置，而 WritePreparedTxn
    //                 // 需要自定义 gc 进行压缩，因此我们也使用它来设置 use_custom_gc_
    //                 //。
    //                 use_custom_gc_(seq_per_batch),
    //                 shutdown_initiated_(false),
    //                 own_sfm_(options.sst_file_manager == nullptr),
    //                 closed_(false),
    //                 atomic_flush_install_cv_(&mutex_),
    //                 blob_callback_(immutable_db_options_.sst_file_manager.get(), &mutex_,
    //                                 &error_handler_, &event_logger_,
    //                                 immutable_db_options_.listeners, dbname_),
    //                 lock_wal_count_(0) {
    //     // !batch_per_trx_ 暗示 seq_per_batch_，因为它仅为
    //     // WriteUnprepared 未设置，而 WriteUnprepared 应使用 seq_per_batch_。
    //     assert(batch_per_txn_ || seq_per_batch_);

    //     // 保留十个左右的文件用于其他用途，其余的交给 TableCache。
    //     // 为“无限”打开文件的设置提供一个大数字。
    //     const int table_cache_size = (mutable_db_options_.max_open_files == -1)
    //                                     ? TableCache::kInfiniteCapacity
    //                                     : mutable_db_options_.max_open_files - 10;
    //     LRUCacheOptions co;
    //     co.capacity = table_cache_size;
    //     co.num_shard_bits = immutable_db_options_.table_cache_numshardbits;
    //     co.metadata_charge_policy = kDontChargeCacheMetadata;
    //     // TODO：处理完测试结果 (prefetch_test) 后，考虑使用非固定种子
    //     co.hash_seed = 0;
    //     table_cache_ = NewLRUCache(co);
    //     SetDbSessionId();
    //     assert(!db_session_id_.empty());

    //     periodic_task_functions_.emplace(PeriodicTaskType::kDumpStats,
    //                                     [this]() { this->DumpStats(); });
    //     periodic_task_functions_.emplace(PeriodicTaskType::kPersistStats,
    //                                     [this]() { this->PersistStats(); });
    //     periodic_task_functions_.emplace(PeriodicTaskType::kFlushInfoLog,
    //                                     [this]() { this->FlushInfoLog(); });
    //     periodic_task_functions_.emplace(
    //         PeriodicTaskType::kRecordSeqnoTime, [this]() {
    //             this->RecordSeqnoToTimeMapping(/*populate_historical_seconds=*/0);
    //         });

    //     versions_.reset(new VersionSet(
    //         dbname_, &immutable_db_options_, file_options_, table_cache_.get(),
    //         write_buffer_manager_, &write_controller_, &block_cache_tracer_,
    //         io_tracer_, db_id_, db_session_id_, options.daily_offpeak_time_utc,
    //         &error_handler_, read_only));
    //     column_family_memtables_.reset(
    //         new ColumnFamilyMemTablesImpl(versions_->GetColumnFamilySet()));

    //     DumpRocksDBBuildVersion(immutable_db_options_.info_log.get());
    //     DumpDBFileSummary(immutable_db_options_, dbname_, db_session_id_);
    //     immutable_db_options_.Dump(immutable_db_options_.info_log.get());
    //     mutable_db_options_.Dump(immutable_db_options_.info_log.get());
    //     DumpSupportInfo(immutable_db_options_.info_log.get());

    //     max_total_wal_size_.store(mutable_db_options_.max_total_wal_size,
    //                                 std::memory_order_relaxed);
    //     if (write_buffer_manager_) {
    //         wbm_stall_.reset(new WBMStallInterface());
    //     }
    // }
        
    } // namespace rocksdb
    
} // namespace latte
