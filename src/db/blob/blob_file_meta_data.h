




#ifndef __LATTE_C_PLUS_BLOB_FILE_META_DATA_H
#define __LATTE_C_PLUS_BLOB_FILE_META_DATA_H

#include <memory>
#include <unordered_set>
#include <cassert>
#include "./shared_blob_file_meta_data.h"

namespace latte
{
    class BlobFileMetaData {
        public:
            using LinkedSsts = std::unordered_set<uint64_t>;

            static std::shared_ptr<BlobFileMetaData> Create(
                std::shared_ptr<SharedBlobFileMetaData> shared_meta,
                LinkedSsts linked_ssts, uint64_t garbage_blob_count,
                uint64_t garbage_blob_bytes) {
                return std::shared_ptr<BlobFileMetaData>(
                    new BlobFileMetaData(std::move(shared_meta), std::move(linked_ssts),
                                        garbage_blob_count, garbage_blob_bytes));
            }

            BlobFileMetaData(const BlobFileMetaData&) = delete;
            BlobFileMetaData& operator=(const BlobFileMetaData&) = delete;

            BlobFileMetaData(BlobFileMetaData&&) = delete;
            BlobFileMetaData& operator=(BlobFileMetaData&&) = delete;

            const std::shared_ptr<SharedBlobFileMetaData>& GetSharedMeta() const {
                return shared_meta_;
            }

            uint64_t GetBlobFileSize() const {
                assert(shared_meta_);
                return shared_meta_->GetBlobFileSize();
            }

            uint64_t GetBlobFileNumber() const {
                assert(shared_meta_);
                return shared_meta_->GetBlobFileNumber();
            }
            uint64_t GetTotalBlobCount() const {
                assert(shared_meta_);
                return shared_meta_->GetTotalBlobCount();
            }
            uint64_t GetTotalBlobBytes() const {
                assert(shared_meta_);
                return shared_meta_->GetTotalBlobBytes();
            }
            const std::string& GetChecksumMethod() const {
                assert(shared_meta_);
                return shared_meta_->GetChecksumMethod();
            }
            const std::string& GetChecksumValue() const {
                assert(shared_meta_);
                return shared_meta_->GetChecksumValue();
            }

            const LinkedSsts& GetLinkedSsts() const { return linked_ssts_; }

            uint64_t GetGarbageBlobCount() const { return garbage_blob_count_; }
            uint64_t GetGarbageBlobBytes() const { return garbage_blob_bytes_; }

            std::string DebugString() const;

            private:
            BlobFileMetaData(std::shared_ptr<SharedBlobFileMetaData> shared_meta,
                            LinkedSsts linked_ssts, uint64_t garbage_blob_count,
                            uint64_t garbage_blob_bytes)
                : shared_meta_(std::move(shared_meta)),
                    linked_ssts_(std::move(linked_ssts)),
                    garbage_blob_count_(garbage_blob_count),
                    garbage_blob_bytes_(garbage_blob_bytes) {
                assert(shared_meta_);
                assert(garbage_blob_count_ <= shared_meta_->GetTotalBlobCount());
                assert(garbage_blob_bytes_ <= shared_meta_->GetTotalBlobBytes());
            }

            std::shared_ptr<SharedBlobFileMetaData> shared_meta_;
            LinkedSsts linked_ssts_;
            uint64_t garbage_blob_count_;
            uint64_t garbage_blob_bytes_;

    };
} // namespace latte


#endif