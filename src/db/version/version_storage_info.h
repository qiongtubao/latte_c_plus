


#ifndef __LATTE_C_PLUS_VERSION_STORAGE_H
#define __LATTE_C_PLUS_VERSION_STORAGE_H



#include "version_edit.h"
#include <assert.h>
#include "blob/blob_files.h"

namespace latte
{
    namespace rocksdb
    {
        class VersionStorageInfo {

            public:
                // REQUIRES: This version has been saved (see VersionBuilder::SaveTo)
                int NumLevelFiles(int level) const {
                    assert(finalized_);
                    return static_cast<int>(files_[level].size());
                }

                // REQUIRES: This version has been saved (see VersionBuilder::SaveTo)
                const std::vector<FileMetaData*>& LevelFiles(int level) const {
                    return files_[level];
                }

                int num_levels() const { return num_levels_; }
                
                const InternalKeyComparator* InternalComparator() const {
                    return internal_comparator_;
                }

            private:
                std::vector<FileMetaData*>* files_;
                bool finalized_;


                const InternalKeyComparator* internal_comparator_;
                const Comparator* user_comparator_;
                int num_levels_;            // Number of levels
                int num_non_empty_levels_;  // Number of levels. Any level larger than it
                                            // is guaranteed to be empty.
                // Per-level max bytes
                std::vector<uint64_t> level_max_bytes_;

                // A short brief metadata of files per level
                // autovector<latte::LevelFilesBrief> level_files_brief_;
                // FileIndexer file_indexer_;
                // Arena arena_;  // Used to allocate space for file_levels_

                CompactionStyle compaction_style_;

                // List of files per level, files in each level are arranged
                // in increasing order of keys
                std::vector<FileMetaData*>* files_;

                // Map of all table files in version. Maps file number to (level, position on
                // level).
                // using FileLocations = UnorderedMap<uint64_t, FileLocation>;
                // FileLocations file_locations_;

                // Vector of blob files in version sorted by blob file number.
                BlobFiles blob_files_;

                // Level that L0 data should be compacted to. All levels < base_level_ should
                // be empty. -1 if it is not level-compaction so it's not applicable.
                int base_level_;

                // Applies to level compaction when
                // `level_compaction_dynamic_level_bytes=true`. All non-empty levels <=
                // lowest_unnecessary_level_ are not needed and will be drained automatically.
                // -1 if there is no unnecessary level,
                int lowest_unnecessary_level_;

                double level_multiplier_;

                // A list for the same set of files that are stored in files_,
                // but files in each level are now sorted based on file
                // size. The file with the largest size is at the front.
                // This vector stores the index of the file from files_.
                std::vector<std::vector<int>> files_by_compaction_pri_;

                // If true, means that files in L0 have keys with non overlapping ranges
                bool level0_non_overlapping_;

                // An index into files_by_compaction_pri_ that specifies the first
                // file that is not yet compacted
                std::vector<int> next_file_to_compact_by_size_;

                // Only the first few entries of files_by_compaction_pri_ are sorted.
                // There is no need to sort all the files because it is likely
                // that on a running system, we need to look at only the first
                // few largest files because a new version is created every few
                // seconds/minutes (because of concurrent compactions).
                static const size_t number_of_files_to_sort_ = 50;

                // This vector contains list of files marked for compaction and also not
                // currently being compacted. It is protected by DB mutex. It is calculated in
                // ComputeCompactionScore(). Used by Leveled and Universal Compaction.
                autovector<std::pair<int, FileMetaData*>> files_marked_for_compaction_;

                autovector<std::pair<int, FileMetaData*>> expired_ttl_files_;

                autovector<std::pair<int, FileMetaData*>>
                    files_marked_for_periodic_compaction_;

                // These files are considered bottommost because none of their keys can exist
                // at lower levels. They are not necessarily all in the same level. The marked
                // ones are eligible for compaction because they contain duplicate key
                // versions that are no longer protected by snapshot. These variables are
                // protected by DB mutex and are calculated in `GenerateBottommostFiles()` and
                // `ComputeBottommostFilesMarkedForCompaction()`.
                autovector<std::pair<int, FileMetaData*>> bottommost_files_;
                autovector<std::pair<int, FileMetaData*>>
                    bottommost_files_marked_for_compaction_;

                autovector<std::pair<int, FileMetaData*>> files_marked_for_forced_blob_gc_;

                // Threshold for needing to mark another bottommost file. Maintain it so we
                // can quickly check when releasing a snapshot whether more bottommost files
                // became eligible for compaction. It's defined as the min of the max nonzero
                // seqnums of unmarked bottommost files.
                SequenceNumber bottommost_files_mark_threshold_ = kMaxSequenceNumber;

                // The minimum sequence number among all the standalone range tombstone files
                // that are marked for compaction. A standalone range tombstone file is one
                // with just one range tombstone.
                SequenceNumber standalone_range_tombstone_files_mark_threshold_ =
                    kMaxSequenceNumber;

                // Monotonically increases as we release old snapshots. Zero indicates no
                // snapshots have been released yet. When no snapshots remain we set it to the
                // current seqnum, which needs to be protected as a snapshot can still be
                // created that references it.
                SequenceNumber oldest_snapshot_seqnum_ = 0;

                // Level that should be compacted next and its compaction score.
                // Score < 1 means compaction is not strictly needed.  These fields
                // are initialized by ComputeCompactionScore.
                // The most critical level to be compacted is listed first
                // These are used to pick the best compaction level
                std::vector<double> compaction_score_;
                std::vector<int> compaction_level_;
                int l0_delay_trigger_count_ = 0;  // Count used to trigger slow down and stop
                                                    // for number of L0 files.

                // Compact cursors for round-robin compactions in each level
                std::vector<InternalKey> compact_cursor_;

                // the following are the sampled temporary stats.
                // the current accumulated size of sampled files.
                uint64_t accumulated_file_size_;
                // the current accumulated size of all raw keys based on the sampled files.
                uint64_t accumulated_raw_key_size_;
                // the current accumulated size of all raw keys based on the sampled files.
                uint64_t accumulated_raw_value_size_;
                // total number of non-deletion entries
                uint64_t accumulated_num_non_deletions_;
                // total number of deletion entries
                uint64_t accumulated_num_deletions_;
                // current number of non_deletion entries
                uint64_t current_num_non_deletions_;
                // current number of deletion entries
                uint64_t current_num_deletions_;
                // current number of file samples
                uint64_t current_num_samples_;
                // Estimated bytes needed to be compacted until all levels' size is down to
                // target sizes.
                uint64_t estimated_compaction_needed_bytes_;

                // Used for computing bottommost files marked for compaction and checking for
                // offpeak time.
                SystemClock* clock_;
                uint32_t bottommost_file_compaction_delay_;

                bool finalized_;

                // If set to true, we will run consistency checks even if RocksDB
                // is compiled in release mode
                bool force_consistency_checks_;

                EpochNumberRequirement epoch_number_requirement_;

                // OffpeakTimeOption offpeak_time_option_;
        };
    } // namespace rocksdb
    
} // namespace latte





#endif