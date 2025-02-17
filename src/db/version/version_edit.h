

#ifndef __LATTE_C_PLUS_VERSION_EDIT_H
#define __LATTE_C_PLUS_VERSION_EDIT_H


#include <string>
#include <memory>
#include "shared/autovector.h"
#include "../types.h"
#include <set>
#include "../format/internal_key.h"
#include "blob/blob_file_addition.h"
#include "blob/blob_file_garbage.h"

namespace latte
{
    namespace leveldb
    {
        class VersionEdit {
            public:
                VersionEdit() { 
                    Clear(); 
                }
                void Clear();
            
            // uint64_t log_number_;
            // uint64_t prev_log_number_;
            // uint64_t next_file_number_;



        };
    } // namespace leveldb

    namespace rocksdb
    {
        using NewFiles = std::vector<std::pair<int, FileMetaData>>;

        using BlobFileAdditions = std::vector<BlobFileAddition>;
        using BlobFileGarbages = std::vector<BlobFileGarbage>;

        constexpr uint64_t kFileNumberMask = 0x3FFFFFFFFFFFFFFF;
        constexpr uint64_t kUnknownOldestAncesterTime = 0;
        constexpr uint64_t kUnknownNewestKeyTime = 0;
        constexpr uint64_t kUnknownFileCreationTime = 0;
        constexpr uint64_t kUnknownEpochNumber = 0;

        struct FileDescriptor {
            // TableReader* table_reader;
            uint64_t packed_number_and_path_id;
            uint64_t file_size;             // File size in bytes
            SequenceNumber smallest_seqno;  // The smallest seqno in this file
            SequenceNumber largest_seqno;   // The largest seqno in this file


            uint64_t GetNumber() const {
                return packed_number_and_path_id & kFileNumberMask;
            }
            uint32_t GetPathId() const {
                return static_cast<uint32_t>(packed_number_and_path_id /
                                            (kFileNumberMask + 1));
            }
            uint64_t GetFileSize() const { return file_size; }
        };

        struct FileMetaData {
            FileDescriptor fd;
            InternalKey smallest;  // Smallest internal key served by table
            InternalKey largest;   // Largest internal key served by table

            // Needs to be disposed when refs becomes 0.
            CacheHandle* table_reader_handle = nullptr;

            // FileSampledStats stats;

            // File size compensated by deletion entry.
            // This is used to compute a file's compaction priority, and is updated in
            // Version::ComputeCompensatedSizes() first time when the file is created or
            // loaded.  After it is updated (!= 0), it is immutable.
            uint64_t compensated_file_size = 0;
            // These values can mutate, but they can only be read or written from
            // single-threaded LogAndApply thread
            uint64_t num_entries =
                0;  // The number of entries, including deletions and range deletions.
            // The number of deletion entries, including range deletions.
            uint64_t num_deletions = 0;
            uint64_t raw_key_size = 0;    // total uncompressed key size.
            uint64_t raw_value_size = 0;  // total uncompressed value size.
            uint64_t num_range_deletions = 0;
            // This is computed during Flush/Compaction, and is added to
            // `compensated_file_size`. Currently, this estimates the size of keys in the
            // next level covered by range tombstones in this file.
            uint64_t compensated_range_deletion_size = 0;


            int refs = 0;  // Reference count

            bool being_compacted = false;       // Is this file undergoing compaction?
            bool init_stats_from_file = false;  // true if the data-entry stats of this
                                                // file has initialized from file.

            bool marked_for_compaction = false;  // True if client asked us nicely to
                                                // compact this file.
            Temperature temperature = Temperature::kUnknown;

            // Used only in BlobDB. The file number of the oldest blob file this SST file
            // refers to. 0 is an invalid value; BlobDB numbers the files starting from 1.
            uint64_t oldest_blob_file_number = kInvalidBlobFileNumber;
            // For flush output file, oldest ancestor time is the oldest key time in the
            // file.  If the oldest key time is not available, flush time is used.
            //
            // For compaction output file, oldest ancestor time is the oldest
            // among all the oldest key time of its input files, since the file could be
            // the compaction output from other SST files, which could in turn be outputs
            // for compact older SST files. If that's not available, creation time of this
            // compaction output file is used.
            //
            // 0 means the information is not available.
            uint64_t oldest_ancester_time = kUnknownOldestAncesterTime;

            // Unix time when the SST file is created.
            uint64_t file_creation_time = kUnknownFileCreationTime;

            // The order of a file being flushed or ingested/imported.
            // Compaction output file will be assigned with the minimum `epoch_number`
            // among input files'.
            // For L0, larger `epoch_number` indicates newer L0 file.
            uint64_t epoch_number = kUnknownEpochNumber;

            // File checksum
            std::string file_checksum = kUnknownFileChecksum;

            // File checksum function name
            std::string file_checksum_func_name = kUnknownFileChecksumFuncName;

            // SST unique id
            UniqueId64x2 unique_id{};

            // Size of the "tail" part of a SST file
            // "Tail" refers to all blocks after data blocks till the end of the SST file
            uint64_t tail_size = 0;

            // Value of the `AdvancedColumnFamilyOptions.persist_user_defined_timestamps`
            // flag when the file is created. Default to true, only when this flag is
            // false, it's explicitly written to Manifest.
            bool user_defined_timestamps_persisted = true;


            FileMetaData() = default;

            FileMetaData(uint64_t file, uint32_t file_path_id, uint64_t file_size,
                        const InternalKey& smallest_key, const InternalKey& largest_key,
                        const SequenceNumber& smallest_seq,
                        const SequenceNumber& largest_seq, bool marked_for_compact,
                        Temperature _temperature, uint64_t oldest_blob_file,
                        uint64_t _oldest_ancester_time, uint64_t _file_creation_time,
                        uint64_t _epoch_number, const std::string& _file_checksum,
                        const std::string& _file_checksum_func_name,
                        UniqueId64x2 _unique_id,
                        const uint64_t _compensated_range_deletion_size,
                        uint64_t _tail_size, bool _user_defined_timestamps_persisted)
                : fd(file, file_path_id, file_size, smallest_seq, largest_seq),
                    smallest(smallest_key),
                    largest(largest_key),
                    compensated_range_deletion_size(_compensated_range_deletion_size),
                    marked_for_compaction(marked_for_compact),
                    temperature(_temperature),
                    oldest_blob_file_number(oldest_blob_file),
                    oldest_ancester_time(_oldest_ancester_time),
                    file_creation_time(_file_creation_time),
                    epoch_number(_epoch_number),
                    file_checksum(_file_checksum),
                    file_checksum_func_name(_file_checksum_func_name),
                    unique_id(std::move(_unique_id)),
                    tail_size(_tail_size),
                    user_defined_timestamps_persisted(_user_defined_timestamps_persisted) {
                // TEST_SYNC_POINT_CALLBACK("FileMetaData::FileMetaData", this);
            }
        };
        // Standard size unique ID, good enough for almost all practical purposes
        using UniqueId64x2 = std::array<uint64_t, 2>;
        // Retrieve the table files added as well as their associated levels.
        using DeletedFiles = std::set<std::pair<int, uint64_t>>;
        class VersionEdit {
            public:
                VersionEdit() {
                    Clear(); 
                }
                void Clear();
                void SetLogNumber(uint64_t num) {
                    has_log_number_ = true;
                    log_number_ = num;
                }
                void SetNextFile(uint64_t num) {
                    has_next_file_number_ = true;
                    next_file_number_ = num;
                }
                void SetLastSequence(SequenceNumber seq) {
                    has_last_sequence_ = true;
                    last_sequence_ = seq;
                }

                void SetColumnFamily(uint32_t column_family_id) {
                    column_family_ = column_family_id;
                }

                // Delete the specified table file from the specified level.
                void DeleteFile(int level, uint64_t file) {
                    deleted_files_.emplace(level, file);
                }

                bool HasLastSequence() const { return has_last_sequence_; }

                // Add the specified table file at the specified level.
                // REQUIRES: "smallest" and "largest" are smallest and largest keys in file
                // REQUIRES: "oldest_blob_file_number" is the number of the oldest blob file
                // referred to by this file if any, kInvalidBlobFileNumber otherwise.
                void AddFile(int level, uint64_t file, uint32_t file_path_id,
                            uint64_t file_size, const InternalKey& smallest,
                            const InternalKey& largest, const SequenceNumber& smallest_seqno,
                            const SequenceNumber& largest_seqno, bool marked_for_compaction,
                            Temperature temperature, uint64_t oldest_blob_file_number,
                            uint64_t oldest_ancester_time, uint64_t file_creation_time,
                            uint64_t epoch_number, const std::string& file_checksum,
                            const std::string& file_checksum_func_name,
                            const UniqueId64x2& unique_id,
                            const uint64_t compensated_range_deletion_size,
                            uint64_t tail_size, bool user_defined_timestamps_persisted) {
                    assert(smallest_seqno <= largest_seqno);
                    new_files_.emplace_back(
                        level,
                        FileMetaData(file, file_path_id, file_size, smallest, largest,
                                    smallest_seqno, largest_seqno, marked_for_compaction,
                                    temperature, oldest_blob_file_number, oldest_ancester_time,
                                    file_creation_time, epoch_number, file_checksum,
                                    file_checksum_func_name, unique_id,
                                    compensated_range_deletion_size, tail_size,
                                    user_defined_timestamps_persisted));
                    files_to_quarantine_.push_back(file);
                    if (!HasLastSequence() || largest_seqno > GetLastSequence()) {
                        SetLastSequence(largest_seqno);
                    }
                }

                // return true on success.
                // `ts_sz` is the size in bytes for the user-defined timestamp contained in
                // a user key. This argument is optional because it's only required for
                // encoding a `VersionEdit` with new SST files to add. It's used to handle the
                // file boundaries: `smallest`, `largest` when
                // `FileMetaData.user_defined_timestamps_persisted` is false. When reading
                // the Manifest file, a mirroring change needed to handle
                // file boundaries are not added to the `VersionEdit.DecodeFrom` function
                // because timestamp size is not available at `VersionEdit` decoding time,
                // it's instead added to `VersionEditHandler::OnNonCfOperation`.
                bool EncodeTo(std::string* dst,
                                std::optional<size_t> ts_sz = std::nullopt) const;

                // Number of edits
                size_t NumEntries() const {
                    return new_files_.size() + deleted_files_.size() +
                        blob_file_additions_.size() + blob_file_garbages_.size() +
                        wal_additions_.size() + !wal_deletion_.IsEmpty();
                }

                // Delete a WAL (either directly deleted or archived).
                // AddWal and DeleteWalsBefore cannot be called on the same VersionEdit.
                void DeleteWalsBefore(WalNumber number) {
                    assert((NumEntries() == 1) == !wal_deletion_.IsEmpty());
                    wal_deletion_ = WalDeletion(number);
                }


            int max_level_ = 0;
            std::string db_id_;
            std::string comparator_;
            uint64_t log_number_ = 0;
            uint64_t prev_log_number_ = 0;
            uint64_t next_file_number_ = 0;
            uint32_t max_column_family_ = 0;
            // The most recent WAL log number that is deleted
            uint64_t min_log_number_to_keep_ = 0;
            SequenceNumber last_sequence_ = 0;
            bool has_db_id_ = false;
            bool has_comparator_ = false;
            bool has_log_number_ = false;
            bool has_prev_log_number_ = false;
            bool has_next_file_number_ = false;
            bool has_max_column_family_ = false;
            bool has_min_log_number_to_keep_ = false;
            bool has_last_sequence_ = false;
            bool has_persist_user_defined_timestamps_ = false;

            // Compaction cursors for round-robin compaction policy
            // CompactCursors compact_cursors_;

            DeletedFiles deleted_files_;
            NewFiles new_files_;

            BlobFileAdditions blob_file_additions_;
            BlobFileGarbages blob_file_garbages_;

            // WalAdditions wal_additions_;
            // WalDeletion wal_deletion_;

            // Each version edit record should have column_family_ set
            // If it's not set, it is default (0)
            uint32_t column_family_ = 0;
            // a version edit can be either column_family add or
            // column_family drop. If it's column family add,
            // it also includes column family name.
            bool is_column_family_drop_ = false;
            bool is_column_family_add_ = false;
            std::string column_family_name_;

            bool is_in_atomic_group_ = false;
            uint32_t remaining_entries_ = 0;

            std::string full_history_ts_low_;
            bool persist_user_defined_timestamps_ = true;

            // Newly created table files and blob files are eligible for deletion if they
            // are not registered as live files after the background jobs creating them
            // have finished. In case committing the VersionEdit containing such changes
            // to manifest encountered an error, we want to quarantine these files from
            // deletion to avoid prematurely deleting files that ended up getting recorded
            // in Manifest as live files.
            // Since table files and blob files share the same file number space, we just
            // record the file number here.
            autovector<uint64_t> files_to_quarantine_;



        };
    } // namespace rocksdb
    
    
} // namespace latte

#endif
