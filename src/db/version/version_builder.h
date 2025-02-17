



#ifndef __LATTE_C_PLUS_VERSION_BUILDER_H
#define __LATTE_C_PLUS_VERSION_BUILDER_H

#include "../column_family/column_family_mem_tables.h"
#include "version.h"
#include "version_edit_handler.h"
#include "../cache/cache_reservation_manager.h"
#include "./base_referenced_version_builder.h"

namespace latte
{
    namespace rocksdb
    {

        class NewestFirstBySeqNo {
            public:
                bool operator()(const FileMetaData* lhs, const FileMetaData* rhs) const {
                    assert(lhs);
                    assert(rhs);

                    if (lhs->fd.largest_seqno != rhs->fd.largest_seqno) {
                        return lhs->fd.largest_seqno > rhs->fd.largest_seqno;
                    }

                    if (lhs->fd.smallest_seqno != rhs->fd.smallest_seqno) {
                        return lhs->fd.smallest_seqno > rhs->fd.smallest_seqno;
                    }

                    // Break ties by file number
                    return lhs->fd.GetNumber() > rhs->fd.GetNumber();
                }
        };

        class NewestFirstByEpochNumber {
            private:
                inline static const NewestFirstBySeqNo seqno_cmp;

            public:
                bool operator()(const FileMetaData* lhs, const FileMetaData* rhs) const {
                    assert(lhs);
                    assert(rhs);

                    if (lhs->epoch_number != rhs->epoch_number) {
                        return lhs->epoch_number > rhs->epoch_number;
                    } else {
                        return seqno_cmp(lhs, rhs);
                    }
                }
        };

        struct LevelState {
            std::unordered_set<uint64_t> deleted_files;
            // Map from file number to file meta data.
            std::unordered_map<uint64_t, FileMetaData*> added_files;
        };


          class BySmallestKey {
            public:
                explicit BySmallestKey(const InternalKeyComparator* cmp) : cmp_(cmp) {}

                bool operator()(const FileMetaData* lhs, const FileMetaData* rhs) const {
                    assert(lhs);
                    assert(rhs);
                    assert(cmp_);

                    const int r = cmp_->Compare(lhs->smallest, rhs->smallest);
                    if (r != 0) {
                        return (r < 0);
                    }

                    // Break ties by file number
                    return (lhs->fd.GetNumber() < rhs->fd.GetNumber());
                }

            private:
                const InternalKeyComparator* cmp_;
        };

            // A class that represents the accumulated changes (like additional garbage or
            // newly linked/unlinked SST files) for a given blob file after applying a
            // series of VersionEdits.
            class BlobFileMetaDataDelta {
            public:
                bool IsEmpty() const {
                return !additional_garbage_count_ && !additional_garbage_bytes_ &&
                        newly_linked_ssts_.empty() && newly_unlinked_ssts_.empty();
                }

                uint64_t GetAdditionalGarbageCount() const {
                    return additional_garbage_count_;
                }

                uint64_t GetAdditionalGarbageBytes() const {
                    return additional_garbage_bytes_;
                }

                const std::unordered_set<uint64_t>& GetNewlyLinkedSsts() const {
                    return newly_linked_ssts_;
                }

                const std::unordered_set<uint64_t>& GetNewlyUnlinkedSsts() const {
                    return newly_unlinked_ssts_;
                }

                void AddGarbage(uint64_t count, uint64_t bytes) {
                    additional_garbage_count_ += count;
                    additional_garbage_bytes_ += bytes;
                }

                void LinkSst(uint64_t sst_file_number) {
                    assert(newly_linked_ssts_.find(sst_file_number) ==
                            newly_linked_ssts_.end());

                    // Reconcile with newly unlinked SSTs on the fly. (Note: an SST can be
                    // linked to and unlinked from the same blob file in the case of a trivial
                    // move.)
                    auto it = newly_unlinked_ssts_.find(sst_file_number);

                    if (it != newly_unlinked_ssts_.end()) {
                        newly_unlinked_ssts_.erase(it);
                    } else {
                        newly_linked_ssts_.emplace(sst_file_number);
                    }
                }

                void UnlinkSst(uint64_t sst_file_number) {
                    assert(newly_unlinked_ssts_.find(sst_file_number) ==
                            newly_unlinked_ssts_.end());

                    // Reconcile with newly linked SSTs on the fly. (Note: an SST can be
                    // linked to and unlinked from the same blob file in the case of a trivial
                    // move.)
                    auto it = newly_linked_ssts_.find(sst_file_number);

                    if (it != newly_linked_ssts_.end()) {
                        newly_linked_ssts_.erase(it);
                    } else {
                        newly_unlinked_ssts_.emplace(sst_file_number);
                    }
                }

            private:
                uint64_t additional_garbage_count_ = 0;
                uint64_t additional_garbage_bytes_ = 0;
                std::unordered_set<uint64_t> newly_linked_ssts_;
                std::unordered_set<uint64_t> newly_unlinked_ssts_;
            };


          class MutableBlobFileMetaData {
            public:
                // To be used for brand new blob files
                explicit MutableBlobFileMetaData(
                    std::shared_ptr<SharedBlobFileMetaData>&& shared_meta)
                    : shared_meta_(std::move(shared_meta)) {}

                // To be used for pre-existing blob files
                explicit MutableBlobFileMetaData(
                    const std::shared_ptr<BlobFileMetaData>& meta)
                    : shared_meta_(meta->GetSharedMeta()),
                    linked_ssts_(meta->GetLinkedSsts()),
                    garbage_blob_count_(meta->GetGarbageBlobCount()),
                    garbage_blob_bytes_(meta->GetGarbageBlobBytes()) {}

                const std::shared_ptr<SharedBlobFileMetaData>& GetSharedMeta() const {
                    return shared_meta_;
                }

                uint64_t GetBlobFileNumber() const {
                    assert(shared_meta_);
                    return shared_meta_->GetBlobFileNumber();
                }

                bool HasDelta() const { return !delta_.IsEmpty(); }

                const std::unordered_set<uint64_t>& GetLinkedSsts() const {
                    return linked_ssts_;
                }

                uint64_t GetGarbageBlobCount() const { return garbage_blob_count_; }

                uint64_t GetGarbageBlobBytes() const { return garbage_blob_bytes_; }

                bool AddGarbage(uint64_t count, uint64_t bytes) {
                    assert(shared_meta_);

                    if (garbage_blob_count_ + count > shared_meta_->GetTotalBlobCount() ||
                        garbage_blob_bytes_ + bytes > shared_meta_->GetTotalBlobBytes()) {
                        return false;
                    }

                    delta_.AddGarbage(count, bytes);

                    garbage_blob_count_ += count;
                    garbage_blob_bytes_ += bytes;

                    return true;
                }

                void LinkSst(uint64_t sst_file_number) {
                    delta_.LinkSst(sst_file_number);

                    assert(linked_ssts_.find(sst_file_number) == linked_ssts_.end());
                    linked_ssts_.emplace(sst_file_number);
                }

                void UnlinkSst(uint64_t sst_file_number) {
                    delta_.UnlinkSst(sst_file_number);

                    assert(linked_ssts_.find(sst_file_number) != linked_ssts_.end());
                    linked_ssts_.erase(sst_file_number);
                }

            private:
                std::shared_ptr<SharedBlobFileMetaData> shared_meta_;
                // Accumulated changes
                BlobFileMetaDataDelta delta_;
                // Resulting state after applying the changes
                BlobFileMetaData::LinkedSsts linked_ssts_;
                uint64_t garbage_blob_count_ = 0;
                uint64_t garbage_blob_bytes_ = 0;
            };



        class VersionBuilderRep {
            public:
                VersionBuilderRep(const FileOptions& file_options, const ImmutableCFOptions* ioptions,
                    TableCache* table_cache, VersionStorageInfo* base_vstorage,
                    VersionSet* version_set,
                    std::shared_ptr<CacheReservationManager> file_metadata_cache_res_mgr,
                    ColumnFamilyData* cfd, VersionEditHandler* version_edit_handler,
                    bool track_found_and_missing_files, bool allow_incomplete_valid_version)
                    : file_options_(file_options),
                        ioptions_(ioptions),
                        table_cache_(table_cache),
                        base_vstorage_(base_vstorage),
                        version_set_(version_set),
                        num_levels_(base_vstorage->num_levels()),
                        has_invalid_levels_(false),
                        level_zero_cmp_by_epochno_(
                            std::make_shared<NewestFirstByEpochNumber>()),
                        level_zero_cmp_by_seqno_(std::make_shared<NewestFirstBySeqNo>()),
                        level_nonzero_cmp_(std::make_shared<BySmallestKey>(
                            base_vstorage_->InternalComparator())),
                        file_metadata_cache_res_mgr_(file_metadata_cache_res_mgr),
                        cfd_(cfd),
                        version_edit_handler_(version_edit_handler),
                        track_found_and_missing_files_(track_found_and_missing_files),
                        allow_incomplete_valid_version_(allow_incomplete_valid_version) {
                    assert(ioptions_);

                    levels_ = new LevelState[num_levels_];
                    if (track_found_and_missing_files_) {
                    assert(cfd_);
                    assert(version_edit_handler_);
                    // `track_found_and_missing_files_` mode used by VersionEditHandlerPIT
                    // assumes the initial base version is valid. For best efforts recovery,
                    // base will be empty. For manifest tailing usage like secondary instance,
                    // they do not allow incomplete version, so the base version in subsequent
                    // catch up attempts should be valid too.
                    valid_version_available_ = true;
                    edited_in_atomic_group_ = false;
                    version_updated_since_last_check_ = false;
                    }
                }

                VersionBuilderRep(const VersionBuilderRep& other)
                    : file_options_(other.file_options_),
                        ioptions_(other.ioptions_),
                        table_cache_(other.table_cache_),
                        base_vstorage_(other.base_vstorage_),
                        version_set_(other.version_set_),
                        num_levels_(other.num_levels_),
                        invalid_level_sizes_(other.invalid_level_sizes_),
                        has_invalid_levels_(other.has_invalid_levels_),
                        table_file_levels_(other.table_file_levels_),
                        updated_compact_cursors_(other.updated_compact_cursors_),
                        level_zero_cmp_by_epochno_(other.level_zero_cmp_by_epochno_),
                        level_zero_cmp_by_seqno_(other.level_zero_cmp_by_seqno_),
                        level_nonzero_cmp_(other.level_nonzero_cmp_),
                        mutable_blob_file_metas_(other.mutable_blob_file_metas_),
                        file_metadata_cache_res_mgr_(other.file_metadata_cache_res_mgr_),
                        cfd_(other.cfd_),
                        version_edit_handler_(other.version_edit_handler_),
                        track_found_and_missing_files_(other.track_found_and_missing_files_),
                        allow_incomplete_valid_version_(other.allow_incomplete_valid_version_),
                        found_files_(other.found_files_),
                        l0_missing_files_(other.l0_missing_files_),
                        non_l0_missing_files_(other.non_l0_missing_files_),
                        intermediate_files_(other.intermediate_files_),
                        missing_blob_files_high_(other.missing_blob_files_high_),
                        missing_blob_files_(other.missing_blob_files_),
                        valid_version_available_(other.valid_version_available_),
                        edited_in_atomic_group_(other.edited_in_atomic_group_),
                        version_updated_since_last_check_(
                            other.version_updated_since_last_check_) {
                    assert(ioptions_);
                    levels_ = new LevelState[num_levels_];
                    for (int level = 0; level < num_levels_; level++) {
                    levels_[level] = other.levels_[level];
                    const auto& added = levels_[level].added_files;
                        for (auto& pair : added) {
                            RefFile(pair.second);
                        }
                    }
                    if (track_found_and_missing_files_) {
                        assert(cfd_);
                        assert(version_edit_handler_);
                    }
                }

                ~VersionBuilderRep() {
                    for (int level = 0; level < num_levels_; level++) {
                        const auto& added = levels_[level].added_files;
                        for (auto& pair : added) {
                            UnrefFile(pair.second);
                        }
                    }

                    delete[] levels_;
                }

                void RefFile(FileMetaData* f) {
                    assert(f);
                    assert(f->refs > 0);
                    f->refs++;
                }

                void UnrefFile(FileMetaData* f) {
                    f->refs--;
                    if (f->refs <= 0) {
                    if (f->table_reader_handle) {
                        assert(table_cache_ != nullptr);
                        // NOTE: have to release in raw cache interface to avoid using a
                        // TypedHandle for FileMetaData::table_reader_handle
                        table_cache_->get_cache().get()->Release(f->table_reader_handle);
                        f->table_reader_handle = nullptr;
                    }

                    if (file_metadata_cache_res_mgr_) {
                        Status s = file_metadata_cache_res_mgr_->UpdateCacheReservation(
                            f->ApproximateMemoryUsage(), false /* increase */);
                        s.PermitUncheckedError();
                    }
                    delete f;
                    }
                }
            public:
                const FileOptions& file_options_;
                const ImmutableCFOptions* const ioptions_;
                TableCache* table_cache_;
                VersionStorageInfo* base_vstorage_;
                VersionSet* version_set_;
                int num_levels_;
                LevelState* levels_;
                // Store sizes of levels larger than num_levels_. We do this instead of
                // storing them in levels_ to avoid regression in case there are no files
                // on invalid levels. The version is not consistent if in the end the files
                // on invalid levels don't cancel out.
                std::unordered_map<int, size_t> invalid_level_sizes_;
                // Whether there are invalid new files or invalid deletion on levels larger
                // than num_levels_.
                bool has_invalid_levels_;
                // Current levels of table files affected by additions/deletions.
                std::unordered_map<uint64_t, int> table_file_levels_;
                // Current compact cursors that should be changed after the last compaction
                std::unordered_map<int, InternalKey> updated_compact_cursors_;
                const std::shared_ptr<const NewestFirstByEpochNumber>
                    level_zero_cmp_by_epochno_;
                const std::shared_ptr<const NewestFirstBySeqNo> level_zero_cmp_by_seqno_;
                const std::shared_ptr<const BySmallestKey> level_nonzero_cmp_;

                // Mutable metadata objects for all blob files affected by the series of
                // version edits.
                std::map<uint64_t, MutableBlobFileMetaData> mutable_blob_file_metas_;

                std::shared_ptr<CacheReservationManager> file_metadata_cache_res_mgr_;

                ColumnFamilyData* cfd_;
                VersionEditHandler* version_edit_handler_;
                bool track_found_and_missing_files_;
                // If false, only a complete Version with all files consisting it found is
                // considered valid. If true, besides complete Version, if the Version is
                // never edited in an atomic group, an incomplete Version with only a suffix
                // of L0 files missing is also considered valid.
                bool allow_incomplete_valid_version_;

                // These are only tracked if `track_found_and_missing_files_` is enabled.

                // The SST files that are found (blob files not included yet).
                std::unordered_set<uint64_t> found_files_;
                // Missing SST files for L0
                std::unordered_set<uint64_t> l0_missing_files_;
                // Missing SST files for non L0 levels
                std::unordered_set<uint64_t> non_l0_missing_files_;
                // Intermediate SST files (blob files not included yet)
                std::vector<std::string> intermediate_files_;
                // The highest file number for all the missing blob files, useful to check
                // if a complete Version is available.
                uint64_t missing_blob_files_high_ = kInvalidBlobFileNumber;
                // Missing blob files, useful to check if only the missing L0 files'
                // associated blob files are missing.
                std::unordered_set<uint64_t> missing_blob_files_;
                // True if all files consisting the Version can be found. Or if
                // `allow_incomplete_valid_version_` is true and the version history is not
                // ever edited in an atomic group, this will be true if only a
                // suffix of L0 SST files and their associated blob files are missing.
                bool valid_version_available_;
                // True if version is ever edited in an atomic group.
                bool edited_in_atomic_group_;

                // Flag to indicate if the Version is updated since last validity check. If no
                // `Apply` call is made between a `Rep`'s construction and a
                // `ValidVersionAvailable` check or between two `ValidVersionAvailable` calls.
                // This flag will be true to indicate the cached validity value can be
                // directly used without a recheck.
                bool version_updated_since_last_check_;

                // End of fields that are only tracked when `track_found_and_missing_files_`
                // is enabled.
        };

        class VersionBuilder {
            public:
                VersionBuilder(const FileOptions& file_options,
                                const ImmutableCFOptions* ioptions, TableCache* table_cache,
                                VersionStorageInfo* base_vstorage, VersionSet* version_set,
                                std::shared_ptr<CacheReservationManager>
                                    file_metadata_cache_res_mgr = nullptr,
                                ColumnFamilyData* cfd = nullptr,
                                VersionEditHandler* version_edit_handler = nullptr,
                                bool track_found_and_missing_files = false,
                                bool allow_incomplete_valid_version = false);
                ~VersionBuilder();

                bool CheckConsistencyForNumLevels();

                Status Apply(const VersionEdit* edit);

                // Save the current Version to the provided `vstorage`.
                Status SaveTo(VersionStorageInfo* vstorage) const;

                // Load all the table handlers for the current Version in the builder.
                Status LoadTableHandlers(InternalStats* internal_stats, int max_threads,
                                        bool prefetch_index_and_filter_in_cache,
                                        bool is_initial_load,
                                        const MutableCFOptions& mutable_cf_options,
                                        size_t max_file_size_for_l0_meta_pin,
                                        const ReadOptions& read_options);

                //============APIs only used by VersionEditHandlerPointInTime ============//

                // Creates a save point for the Version that has been built so far. Subsequent
                // VersionEdits applied to the builder will not affect the Version in this
                // save point. VersionBuilder currently only supports creating one save point,
                // so when `CreateOrReplaceSavePoint` is called again, the previous save point
                // is cleared. `ClearSavePoint` can be called explicitly to clear
                // the save point too.
                void CreateOrReplaceSavePoint();

                // The builder can find all the files to build a `Version`. Or if
                // `allow_incomplete_valid_version_` is true and the version history is never
                // edited in an atomic group, and only a suffix of L0 SST files and their
                // associated blob files are missing.
                // From the users' perspective, missing a suffix of L0 files means missing the
                // user's most recently written data. So the remaining available files still
                // presents a valid point in time view, although for some previous time.
                // This validity check result will be cached and reused if the Version is not
                // updated between two validity checks.
                bool ValidVersionAvailable();

                bool HasMissingFiles() const;

                // When applying a sequence of VersionEdit, intermediate files are the ones
                // that are added and then deleted. The caller should clear this intermediate
                // files tracking after calling this API. So that the tracking for subsequent
                // VersionEdits can start over with a clean state.
                std::vector<std::string>& GetAndClearIntermediateFiles();

                // Clearing all the found files in this Version.
                void ClearFoundFiles();

                // Save the Version in the save point to the provided `vstorage`.
                // Non-OK status will be returned if there is not a valid save point.
                Status SaveSavePointTo(VersionStorageInfo* vstorage) const;

                // Load all the table handlers for the Version in the save point.
                // Non-OK status will be returned if there is not a valid save point.
                Status LoadSavePointTableHandlers(InternalStats* internal_stats,
                                                    int max_threads,
                                                    bool prefetch_index_and_filter_in_cache,
                                                    bool is_initial_load,
                                                    const MutableCFOptions& mutable_cf_options,
                                                    size_t max_file_size_for_l0_meta_pin,
                                                    const ReadOptions& read_options);

                void ClearSavePoint();

                //======= End of APIs only used by VersionEditPointInTime==========//

            private:
                class Rep;
                std::unique_ptr<Rep> savepoint_;
                std::unique_ptr<Rep> rep_;
        };

        // // A wrapper of version builder which references the current version in
        // // constructor and unref it in the destructor.
        // // Both of the constructor and destructor need to be called inside DB Mutex.
        // class BaseReferencedVersionBuilder {
        //     public:
        //         explicit BaseReferencedVersionBuilder(
        //             ColumnFamilyData* cfd, VersionEditHandler* version_edit_handler = nullptr,
        //             bool track_found_and_missing_files = false,
        //             bool allow_incomplete_valid_version = false);
        //         BaseReferencedVersionBuilder(
        //             ColumnFamilyData* cfd, Version* v,
        //             VersionEditHandler* version_edit_handler = nullptr,
        //             bool track_found_and_missing_files = false,
        //             bool allow_incomplete_valid_version = false);
        //         ~BaseReferencedVersionBuilder();
        //         VersionBuilder* version_builder() const { return version_builder_.get(); }

        //     private:
        //         std::unique_ptr<VersionBuilder> version_builder_;
        //         Version* version_;
        // };
    } // namespace rocksdb
    
} // namespace latte




#endif