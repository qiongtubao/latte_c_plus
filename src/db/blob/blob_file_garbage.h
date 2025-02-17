

#ifndef __LATTE_C_PLUS_BLOB_FILE_GARBAGE_H
#define __LATTE_C_PLUS_BLOB_FILE_GARBAGE_H


#include <cstdint>
#include "blob_constants.h"
#include <iosfwd>
#include <string>
namespace latte
{
    namespace rocksdb
    {
        class BlobFileGarbage {
            public:
                BlobFileGarbage() = default;
                BlobFileGarbage(uint64_t blob_file_number, uint64_t garbage_blob_count,
                  uint64_t garbage_blob_bytes)
                : blob_file_number_(blob_file_number),
                    garbage_blob_count_(garbage_blob_count),
                    garbage_blob_bytes_(garbage_blob_bytes) {}
            private:
                enum CustomFieldTags : uint32_t;
                uint64_t blob_file_number_ = kInvalidBlobFileNumber;
                uint64_t garbage_blob_count_ = 0;
                uint64_t garbage_blob_bytes_ = 0;
        };
        bool operator==(const BlobFileGarbage& lhs, const BlobFileGarbage& rhs);
        bool operator!=(const BlobFileGarbage& lhs, const BlobFileGarbage& rhs);

        std::ostream& operator<<(std::ostream& os,
                                const BlobFileGarbage& blob_file_garbage);
    } // namespace rocksdb
    
} // namespace latte


#endif