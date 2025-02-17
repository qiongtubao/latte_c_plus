
#ifndef __LATTE_C_PLUS_FORMAT_CONFIG_H
#define __LATTE_C_PLUS_FORMAT_CONFIG_H

#include "../port/port_posix.h"

namespace latte
{
    
    namespace leveldb
    {
        namespace config
        {

            static const int kNumLevels = 7;

            // Level-0 compaction is started when we hit this many files.
            static const int kL0_CompactionTrigger = 4;

            // Soft limit on number of level-0 files.  We slow down writes at this point.
            static const int kL0_SlowdownWritesTrigger = 8;

            // Maximum number of level-0 files.  We stop writes at this point.
            static const int kL0_StopWritesTrigger = 12;

            // Maximum level to which a new compacted memtable is pushed if it
            // does not create overlap.  We try to push to level 2 to avoid the
            // relatively expensive level 0=>1 compactions and to avoid some
            // expensive manifest file operations.  We do not push all the way to
            // the largest level since that can generate a lot of wasted disk
            // space if the same key space is being repeatedly overwritten.
            static const int kMaxMemCompactLevel = 2;

            // Approximate gap in bytes between samples of data read during iteration.
            static const int kReadBytesPeriod = 1048576;

            
        } // namespace config
        

        
       

    } // namespace leveldb
    

    namespace rocksdb
    {
        constexpr uint64_t kNumInternalBytes = 8;
        // 值类型被编码为内部键的最后一个组件。
        // 请勿更改这些枚举值：它们嵌入在磁盘上
        // 数据结构中。
        // 值类型的最高位需要保留给 SST 表
        // 以便它们进行更灵活的编码。
        enum ValueType : unsigned char {
            kTypeDeletion = 0x0,
            kTypeValue = 0x1,
            kTypeMerge = 0x2,
            kTypeLogData = 0x3,               // WAL only.
            kTypeColumnFamilyDeletion = 0x4,  // WAL only.
            kTypeColumnFamilyValue = 0x5,     // WAL only.
            kTypeColumnFamilyMerge = 0x6,     // WAL only.
            kTypeSingleDeletion = 0x7,
            kTypeColumnFamilySingleDeletion = 0x8,  // WAL only.
            kTypeBeginPrepareXID = 0x9,             // WAL only.
            kTypeEndPrepareXID = 0xA,               // WAL only.
            kTypeCommitXID = 0xB,                   // WAL only.
            kTypeRollbackXID = 0xC,                 // WAL only.
            kTypeNoop = 0xD,                        // WAL only.
            kTypeColumnFamilyRangeDeletion = 0xE,   // WAL only.
            kTypeRangeDeletion = 0xF,               // meta block
            kTypeColumnFamilyBlobIndex = 0x10,      // Blob DB only
            kTypeBlobIndex = 0x11,                  // Blob DB only
            // 当准备好的记录也持久保存在数据库中时，我们使用不同的
            // 记录。这是为了确保 WritePolicy
            // 生成的 WAL 不会被另一个错误读取，从而导致数据
            // 不一致。
            kTypeBeginPersistedPrepareXID = 0x12,  // WAL only.
            // 类似于 kTypeBeginPersistedPrepareXID，这是为了确保 WriteUnprepared 写策略生成的 WAL
            // 不会被另一个错误读取。
            kTypeBeginUnprepareXID = 0x13,  // WAL only.
            kTypeDeletionWithTimestamp = 0x14,
            kTypeCommitXIDAndTimestamp = 0x15,  // WAL only
            kTypeWideColumnEntity = 0x16,
            kTypeColumnFamilyWideColumnEntity = 0x17,     // WAL only
            kTypeValuePreferredSeqno = 0x18,              // Value with a unix write time
            kTypeColumnFamilyValuePreferredSeqno = 0x19,  // WAL only
            kTypeMaxValid,    // Should be after the last valid type, only used for
                                // validation
            kMaxValue = 0x7F  // Not used for storing records.
        };

        // 我们在底部留出 8 位空白，以便类型和序列号
        // 可以一起打包成 64 位。
        static const SequenceNumber kMaxSequenceNumber = ((0x1ull << 56) - 1);

        // kValueTypeForSeek defines the ValueType that should be passed when
        // constructing a ParsedInternalKey object for seeking to a particular
        // sequence number (since we sort sequence numbers in decreasing order
        // and the value type is embedded as the low 8 bits in the sequence
        // number in internal keys, we need to use the highest-numbered
        // ValueType, not the lowest).
        const ValueType kValueTypeForSeek = kTypeValuePreferredSeqno;
        const ValueType kValueTypeForSeekForPrev = kTypeDeletion;

        inline void EncodeFixed32(char* buf, uint32_t value) {
            if (port::kLittleEndian) {
                memcpy(buf, &value, sizeof(value));
            } else {
                buf[0] = value & 0xff;
                buf[1] = (value >> 8) & 0xff;
                buf[2] = (value >> 16) & 0xff;
                buf[3] = (value >> 24) & 0xff;
            }
        }

        inline void EncodeFixed64(char* buf, uint64_t value) {
            if (port::kLittleEndian) {
                memcpy(buf, &value, sizeof(value));
            } else {
                buf[0] = value & 0xff;
                buf[1] = (value >> 8) & 0xff;
                buf[2] = (value >> 16) & 0xff;
                buf[3] = (value >> 24) & 0xff;
                buf[4] = (value >> 32) & 0xff;
                buf[5] = (value >> 40) & 0xff;
                buf[6] = (value >> 48) & 0xff;
                buf[7] = (value >> 56) & 0xff;
            }
        }

        inline uint32_t DecodeFixed32(const char* ptr) {
            if (port::kLittleEndian) {
                // Load the raw bytes
                uint32_t result;
                memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
                return result;
            } else {
                return ((static_cast<uint32_t>(static_cast<unsigned char>(ptr[0]))) |
                        (static_cast<uint32_t>(static_cast<unsigned char>(ptr[1])) << 8) |
                        (static_cast<uint32_t>(static_cast<unsigned char>(ptr[2])) << 16) |
                        (static_cast<uint32_t>(static_cast<unsigned char>(ptr[3])) << 24));
            }
        }

        // Lower-level versions of Get... that read directly from a character buffer
        // without any bounds checking.

        inline uint64_t DecodeFixed64(const char* ptr) {
            if (port::kLittleEndian) {
                // Load the raw bytes
                uint64_t result;
                memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
                return result;
            } else {
                uint64_t lo = DecodeFixed32(ptr);
                uint64_t hi = DecodeFixed32(ptr + 4);
                return (hi << 32) | lo;
            }
        }

        // Checks whether a type is an inline value type
        // (i.e. a type used in memtable skiplist and sst file datablock).
        inline bool IsValueType(ValueType t) {
            return t <= kTypeMerge || kTypeSingleDeletion == t || kTypeBlobIndex == t ||
                    kTypeDeletionWithTimestamp == t || kTypeWideColumnEntity == t ||
                    kTypeValuePreferredSeqno == t;
        }

        char* EncodeVarint32(char* dst, uint32_t v);

        inline int VarintLength(uint64_t v) {
            int len = 1;
            while (v >= 128) {
                v >>= 7;
                len++;
            }
            return len;
        }
    } // namespace rocksdb

} // namespace lattena

#endif