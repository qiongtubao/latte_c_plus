

#ifndef __LATTE_C_PLUS_FILENAME_H
#define __LATTE_C_PLUS_FILENAME_H

#pragma once


#include <string>
#include "io/types.h"
#include "slice/slice.h"
#include "slice/string_util.h"
#include "io/fs_directory.h"
#include <stdlib.h>

namespace latte
{
    namespace rocksdb
    {
        enum WalFileType {
            /* Indicates that WAL file is in archive directory. WAL files are moved from
            * the main db directory to archive directory once they are not live and stay
            * there until cleaned up. Files are cleaned depending on archive size
            * (Options::WAL_size_limit_MB) and time since last cleaning
            * (Options::WAL_ttl_seconds).
            */
            kArchivedLogFile = 0,

            /* Indicates that WAL file is live and resides in the main db directory */
            kAliveLogFile = 1
        };
        const std::string kCurrentFileName = "CURRENT";
        const std::string kOptionsFileNamePrefix = "OPTIONS-";
        const std::string kTempFileNameSuffix = "dbtmp";

        static const std::string kRocksDbTFileExt = "sst";
        static const std::string kLevelDbTFileExt = "ldb";
        static const std::string kRocksDBBlobFileExt = "blob";
        static const std::string kArchivalDirName = "archive";
        std::string LockFileName(const std::string& dbname) { return dbname + "/LOCK"; }
        //当前文件内存储的是清单
        std::string CurrentFileName(const std::string& dbname) {
            return dbname + "/" + kCurrentFileName;
        }

        // Owned filenames have the form:
        //    dbname/IDENTITY
        //    dbname/CURRENT
        //    dbname/LOCK
        //    dbname/<info_log_name_prefix>
        //    dbname/<info_log_name_prefix>.old.[0-9]+
        //    dbname/MANIFEST-[0-9]+
        //    dbname/[0-9]+.(log|sst|blob)
        //    dbname/METADB-[0-9]+
        //    dbname/OPTIONS-[0-9]+
        //    dbname/OPTIONS-[0-9]+.dbtmp
        //    Disregards / at the beginning
        bool ParseFileName(const std::string& fname, uint64_t* number, FileType* type,
                        WalFileType* log_type = nullptr) {
            return ParseFileName(fname, number, "", type, log_type);
        }
        bool ParseFileName(const std::string& fname, uint64_t* number,
                   const Slice& info_log_name_prefix, FileType* type,
                   WalFileType* log_type = nullptr) {
            Slice rest(fname);
            if (fname.length() > 1 && fname[0] == '/') {
                rest.remove_prefix(1);
            }
            if (rest == "IDENTITY") {
                *number = 0;
                *type = kIdentityFile;
            } else if (rest == "CURRENT") {
                *number = 0;
                *type = kCurrentFile;
            } else if (rest == "LOCK") {
                *number = 0;
                *type = kDBLockFile;
            } else if (info_log_name_prefix.size() > 0 &&
                        rest.starts_with(info_log_name_prefix)) {
                rest.remove_prefix(info_log_name_prefix.size());
                if (rest == "" || rest == ".old") {
                *number = 0;
                *type = kInfoLogFile;
                } else if (rest.starts_with(".old.")) {
                uint64_t ts_suffix;
                // sizeof also counts the trailing '\0'.
                rest.remove_prefix(sizeof(".old.") - 1);
                if (!ConsumeDecimalNumber(&rest, &ts_suffix)) {
                    return false;
                }
                *number = ts_suffix;
                *type = kInfoLogFile;
                }
            } else if (rest.starts_with("MANIFEST-")) {
                rest.remove_prefix(strlen("MANIFEST-"));
                uint64_t num;
                if (!ConsumeDecimalNumber(&rest, &num)) {
                return false;
                }
                if (!rest.empty()) {
                return false;
                }
                *type = kDescriptorFile;
                *number = num;
            } else if (rest.starts_with("METADB-")) {
                rest.remove_prefix(strlen("METADB-"));
                uint64_t num;
                if (!ConsumeDecimalNumber(&rest, &num)) {
                return false;
                }
                if (!rest.empty()) {
                return false;
                }
                *type = kMetaDatabase;
                *number = num;
            } else if (rest.starts_with(kOptionsFileNamePrefix)) {
                uint64_t ts_suffix;
                bool is_temp_file = false;
                rest.remove_prefix(kOptionsFileNamePrefix.size());
                const std::string kTempFileNameSuffixWithDot =
                    std::string(".") + kTempFileNameSuffix;
                if (rest.ends_with(kTempFileNameSuffixWithDot)) {
                rest.remove_suffix(kTempFileNameSuffixWithDot.size());
                is_temp_file = true;
                }
                if (!ConsumeDecimalNumber(&rest, &ts_suffix)) {
                return false;
                }
                *number = ts_suffix;
                *type = is_temp_file ? kTempFile : kOptionsFile;
            } else {
                // Avoid strtoull() to keep filename format independent of the
                // current locale
                bool archive_dir_found = false;
                if (rest.starts_with(kArchivalDirName)) {
                if (rest.size() <= kArchivalDirName.size()) {
                    return false;
                }
                rest.remove_prefix(kArchivalDirName.size() +
                                    1);  // Add 1 to remove / also
                if (log_type) {
                    *log_type = kArchivedLogFile;
                }
                archive_dir_found = true;
                }
                uint64_t num;
                if (!ConsumeDecimalNumber(&rest, &num)) {
                return false;
                }
                if (rest.size() <= 1 || rest[0] != '.') {
                return false;
                }
                rest.remove_prefix(1);

                Slice suffix = rest;
                if (suffix == Slice("log")) {
                *type = kWalFile;
                if (log_type && !archive_dir_found) {
                    *log_type = kAliveLogFile;
                }
                } else if (archive_dir_found) {
                return false;  // Archive dir can contain only log files
                } else if (suffix == Slice(kRocksDbTFileExt) ||
                        suffix == Slice(kLevelDbTFileExt)) {
                *type = kTableFile;
                } else if (suffix == Slice(kRocksDBBlobFileExt)) {
                *type = kBlobFile;
                } else if (suffix == Slice(kTempFileNameSuffix)) {
                *type = kTempFile;
                } else {
                return false;
                }
                *number = num;
            }
            return true;
        }
        // Return the name of the descriptor file for the db named by
        // "dbname" and the specified incarnation number.  The result will be
        // prefixed with "dbname".
        std::string DescriptorFileName(const std::string& dbname, uint64_t number);

        std::string DescriptorFileName(uint64_t number);

        // Return the name of the Identity file which stores a unique number for the db
        // that will get regenerated if the db loses all its data and is recreated fresh
        // either from a backup-image or empty
        std::string IdentityFileName(const std::string& dbname);

        // Make the IDENTITY file for the db
        Status SetIdentityFile(const WriteOptions& write_options, Env* env,
                            const std::string& dbname, Temperature temp,
                            const std::string& db_id = {});

        // 使当前文件指向具有指定编号的描述符文件。如果成功，
        // 并且 dir_contains_current_file 不为 nullptr，
        // 则该函数将 fsync 包含当前文件的目录。
        IOStatus SetCurrentFile(const WriteOptions& write_options, FileSystem* fs,
                                const std::string& dbname, uint64_t descriptor_number,
                                Temperature temp,
                                FSDirectory* dir_contains_current_file);
                                
        // Sync manifest file `file`.
        IOStatus SyncManifest(const ImmutableDBOptions* db_options,
                            const WriteOptions& write_options,
                            WritableFileWriter* file);

        // Return the name of the log file with the specified number
        // in the db named by "dbname".  The result will be prefixed with
        // "dbname".
        std::string LogFileName(const std::string& dbname, uint64_t number);

        std::string LogFileName(uint64_t number);

        // Return a options file name given the "dbname" and file number.
        // Format:  OPTIONS-[number].dbtmp
        std::string OptionsFileName(const std::string& dbname, uint64_t file_num);
        std::string OptionsFileName(uint64_t file_num);
        
        std::string NormalizePath(const std::string& path);

    }; // namespace rocksdb
    
} // namespace latte

#endif
