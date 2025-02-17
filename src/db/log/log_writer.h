

#ifndef __LATTE_C_PLUS_ROCKSDB_LOG_WRITER_H
#define __LATTE_C_PLUS_ROCKSDB_LOG_WRITER_H


#include "../write_thread/writable_file_writer.h"
#include "options/write_options.h"
#include "compaction/compression_type.h" 

namespace latte
{
    namespace rocksdb
    {
        class LogWriter {
            public:
                // Create a writer that will append data to "*dest".
                // "*dest" must be initially empty.
                // "*dest" must remain live while this Writer is in use.
                explicit LogWriter(std::unique_ptr<WritableFileWriter>&& dest,
                                uint64_t log_number, bool recycle_log_files,
                                bool manual_flush = false,
                                CompressionType compressionType = kNoCompression);
                // No copying allowed
                LogWriter(const LogWriter&) = delete;
                void operator=(const LogWriter&) = delete;

                ~LogWriter();

                IOStatus AddRecord(const WriteOptions& write_options, const Slice& slice);
                IOStatus AddCompressionTypeRecord(const WriteOptions& write_options);

                // If there are column families in `cf_to_ts_sz` not included in
                // `recorded_cf_to_ts_sz_` and its user-defined timestamp size is non-zero,
                // adds a record of type kUserDefinedTimestampSizeType or
                // kRecyclableUserDefinedTimestampSizeType for these column families.
                // This timestamp size record applies to all subsequent records.
                IOStatus MaybeAddUserDefinedTimestampSizeRecord(
                    const WriteOptions& write_options,
                    const UnorderedMap<uint32_t, size_t>& cf_to_ts_sz);

                WritableFileWriter* file() { return dest_.get(); }
                const WritableFileWriter* file() const { return dest_.get(); }

                uint64_t get_log_number() const { return log_number_; }

                IOStatus WriteBuffer(const WriteOptions& write_options);

                IOStatus Close(const WriteOptions& write_options);

                // If closing the writer through file(), call this afterwards to modify
                // this object's state to reflect that. Returns true if the destination file
                // has been closed. If it hasn't been closed, returns false with no change.
                bool PublishIfClosed();

                bool BufferIsEmpty();

                size_t TEST_block_offset() const { return block_offset_; }

                private:
                std::unique_ptr<WritableFileWriter> dest_;
                size_t block_offset_;  // Current offset in block
                uint64_t log_number_;
                bool recycle_log_files_;
                int header_size_;

                // crc32c values for all supported record types.  These are
                // pre-computed to reduce the overhead of computing the crc of the
                // record type stored in the header.
                uint32_t type_crc_[kMaxRecordType + 1];

                IOStatus EmitPhysicalRecord(const WriteOptions& write_options,
                                            RecordType type, const char* ptr, size_t length);

                // If true, it does not flush after each write. Instead it relies on the upper
                // layer to manually does the flush by calling ::WriteBuffer()
                bool manual_flush_;

                // Compression Type
                CompressionType compression_type_;
                StreamingCompress* compress_;
                // Reusable compressed output buffer
                std::unique_ptr<char[]> compressed_buffer_;

                // The recorded user-defined timestamp size that have been written so far.
                // Since the user-defined timestamp size cannot be changed while the DB is
                // running, existing entry in this map cannot be updated.
                UnorderedMap<uint32_t, size_t> recorded_cf_to_ts_sz_;
        };
    } // namespace rocksdb


    namespace leveldb
    {
        
    } // namespace leveldb
    
    
} // namespace latte


#endif