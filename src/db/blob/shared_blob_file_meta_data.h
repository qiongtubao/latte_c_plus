


#ifndef __LATTE_C_PLUS_SHARED_BLOB_FILE_META_DATA_H
#define __LATTE_C_PLUS_SHARED_BLOB_FILE_META_DATA_H

#include <memory>
#include <string>
#include <cassert>

namespace latte
{
    class SharedBlobFileMetaData {
        public:
            static std::shared_ptr<SharedBlobFileMetaData> Create(
                uint64_t blob_file_number, uint64_t total_blob_count,
                uint64_t total_blob_bytes, std::string checksum_method,
                std::string checksum_value) {
                return std::shared_ptr<SharedBlobFileMetaData>(new SharedBlobFileMetaData(
                    blob_file_number, total_blob_count, total_blob_bytes,
                    std::move(checksum_method), std::move(checksum_value)));
            }

            template <typename Deleter>
            static std::shared_ptr<SharedBlobFileMetaData> Create(
                uint64_t blob_file_number, uint64_t total_blob_count,
                uint64_t total_blob_bytes, std::string checksum_method,
                std::string checksum_value, Deleter deleter) {
                return std::shared_ptr<SharedBlobFileMetaData>(
                    new SharedBlobFileMetaData(blob_file_number, total_blob_count,
                                            total_blob_bytes, std::move(checksum_method),
                                            std::move(checksum_value)),
                    deleter);
            }

            SharedBlobFileMetaData(const SharedBlobFileMetaData&) = delete;
            SharedBlobFileMetaData& operator=(const SharedBlobFileMetaData&) = delete;

            SharedBlobFileMetaData(SharedBlobFileMetaData&&) = delete;
            SharedBlobFileMetaData& operator=(SharedBlobFileMetaData&&) = delete;

            uint64_t GetBlobFileSize() const;
            uint64_t GetBlobFileNumber() const { return blob_file_number_; }
            uint64_t GetTotalBlobCount() const { return total_blob_count_; }
            uint64_t GetTotalBlobBytes() const { return total_blob_bytes_; }
            const std::string& GetChecksumMethod() const { return checksum_method_; }
            const std::string& GetChecksumValue() const { return checksum_value_; }

            std::string DebugString() const;

            private:
            SharedBlobFileMetaData(uint64_t blob_file_number, uint64_t total_blob_count,
                                    uint64_t total_blob_bytes, std::string checksum_method,
                                    std::string checksum_value)
                : blob_file_number_(blob_file_number),
                    total_blob_count_(total_blob_count),
                    total_blob_bytes_(total_blob_bytes),
                    checksum_method_(std::move(checksum_method)),
                    checksum_value_(std::move(checksum_value)) {
                assert(checksum_method_.empty() == checksum_value_.empty());
            }

            uint64_t blob_file_number_;
            uint64_t total_blob_count_;
            uint64_t total_blob_bytes_;
            std::string checksum_method_;
            std::string checksum_value_;
    }; 
} // namespace latte


#endif