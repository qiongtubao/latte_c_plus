


#include "version_set.h"
#include "env/env.h"
#include "../filename.h"
#include "io/sequential_file_reader.h"
#include "io/fs_sequential_file.h"
#include "../log/log_reporter.h"
#include "../log/log_reader.h"
#include "version_edit_handler.h"


namespace latte
{
    namespace rocksdb
    {
        Status VersionSet::GetCurrentManifestPath(const std::string& dbname,
                                          FileSystem* fs, bool is_retry,
                                          std::string* manifest_path,
                                          uint64_t* manifest_file_number) {
            assert(fs != nullptr);
            assert(manifest_path != nullptr);
            assert(manifest_file_number != nullptr);

            IOOptions opts;
            std::string fname;
            if (is_retry) {//重试
                opts.verify_and_reconstruct_read = true; //验证并重建读取
            }
            //读取current 文件内容 
            Status s = ReadFileToString(fs, CurrentFileName(dbname), opts, &fname);
            if (!s.ok()) { //失败返回错误
                return s;
            }
            if (fname.empty() || fname.back() != '\n') { //无文件或者格式不对
                return Status::Corruption("CURRENT file does not end with newline");
            }
            // 删除'\n'
            fname.resize(fname.size() - 1); 
            FileType type;
            bool parse_ok = ParseFileName(fname, manifest_file_number, &type); //解析文件类型
            if (!parse_ok || type != kDescriptorFile) { //解析失败或者不是描述文件
                return Status::Corruption("CURRENT file corrupted");
            }
            *manifest_path = dbname; //设置目录
            if (dbname.back() != '/') {
                manifest_path->push_back('/'); //追加‘/’
            }
            manifest_path->append(fname); //追加描述文件名返回
            return Status::OK();
        }
        Status VersionSet::Recover(
            const std::vector<ColumnFamilyDescriptor>& column_families, bool read_only,
                std::string* db_id, bool no_error_if_files_missing, bool is_retry,
                Status* log_status) {
            const ReadOptions read_options(Env::IOActivity::kDBOpen);
            // 文件
            std::string manifest_path;
            // 读取“当前”文件，其中包含指向当前清单的指针
            Status s = GetCurrentManifestPath(dbname_, fs_.get(), is_retry,
                                                &manifest_path, &manifest_file_number_);
            if (!s.ok()) { //读取失败 返回失败
                return s;
            }

            ROCKS_LOG_INFO(db_options_->info_log, "Recovering from manifest file: %s\n",
                            manifest_path.c_str());
            //顺序文件读取器
            std::unique_ptr<SequentialFileReader> manifest_file_reader;
            {
                std::unique_ptr<FSSequentialFile> manifest_file;
                s = fs_->NewSequentialFile(manifest_path,
                                        fs_->OptimizeForManifestRead(file_options_),
                                        &manifest_file, nullptr);
                if (!s.ok()) {
                    return s;
                }
                manifest_file_reader.reset(new SequentialFileReader(
                    std::move(manifest_file), manifest_path,
                    db_options_->log_readahead_size, io_tracer_, db_options_->listeners,
                    /*rate_limiter=*/nullptr, is_retry));
            }
            // TEST_SYNC_POINT("VersionSet::Recover:StartManifestRead");

            uint64_t current_manifest_file_size = 0;
            uint64_t log_number = 0;
            {
                LogReporter reporter;
                Status log_read_status;
                reporter.status = &log_read_status;
                LogReader reader(nullptr, std::move(manifest_file_reader), &reporter,
                                true /* checksum */, 0 /* log_number */);
                VersionEditHandler handler(
                    read_only, column_families, const_cast<VersionSet*>(this),
                    /*track_found_and_missing_files=*/false, no_error_if_files_missing,
                    io_tracer_, read_options, /*allow_incomplete_valid_version=*/false,
                    EpochNumberRequirement::kMightMissing);
                handler.Iterate(reader, &log_read_status);
                s = handler.status();
                if (s.ok()) {
                log_number = handler.GetVersionEditParams().GetLogNumber();
                current_manifest_file_size = reader.GetReadOffset();
                assert(current_manifest_file_size != 0);
                handler.GetDbId(db_id);
                }
                if (s.ok()) {
                RecoverEpochNumbers();
                }
                if (log_status) {
                *log_status = log_read_status;
                }
            }

            if (s.ok()) {
                manifest_file_size_ = current_manifest_file_size;
                ROCKS_LOG_INFO(
                    db_options_->info_log,
                    "Recovered from manifest file:%s succeeded,"
                    "manifest_file_number is %" PRIu64 ", next_file_number is %" PRIu64
                    ", last_sequence is %" PRIu64 ", log_number is %" PRIu64
                    ",prev_log_number is %" PRIu64 ",max_column_family is %" PRIu32
                    ",min_log_number_to_keep is %" PRIu64 "\n",
                    manifest_path.c_str(), manifest_file_number_, next_file_number_.load(),
                    last_sequence_.load(), log_number, prev_log_number_,
                    column_family_set_->GetMaxColumnFamily(), min_log_number_to_keep());

                for (auto cfd : *column_family_set_) {
                if (cfd->IsDropped()) {
                    continue;
                }
                ROCKS_LOG_INFO(db_options_->info_log,
                                "Column family [%s] (ID %" PRIu32
                                "), log number is %" PRIu64 "\n",
                                cfd->GetName().c_str(), cfd->GetID(), cfd->GetLogNumber());
                }
            }

            return s;
    
    
        }



    } // namespace rocksdb
    
} // namespace latte
