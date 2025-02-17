

#ifndef __LATTE_C_PLUS_WRITE_BATCH_H
#define __LATTE_C_PLUS_WRITE_BATCH_H

#include "write_batch_base.h"
#include "../checksum/protection_info.h"

namespace latte
{
    namespace leveldb
    {
        class WriteBatch {

        };
    } // namespace leveldb

    namespace rocksdb
    {
        struct SavePoint {
            size_t size;     // size of rep_
            uint32_t count;  // count of elements in rep_
            uint32_t content_flags;

            SavePoint() : size(0), count(0), content_flags(0) {}

            SavePoint(size_t _size, uint32_t _count, uint32_t _flags)
                : size(_size), count(_count), content_flags(_flags) {}

            void clear() {
                size = 0;
                count = 0;
                content_flags = 0;
            }

            bool is_cleared() const { return (size | count | content_flags) == 0; }
        };
        struct SavePoints {
            std::stack<SavePoint, autovector<SavePoint>> stack;
        };
        class WriteBatch : public WriteBatchBase {
            public:
                struct ProtectionInfo {
                    // `WriteBatch` usually doesn't contain a huge number of keys so protecting
                    // with a fixed, non-configurable eight bytes per key may work well enough.
                    autovector<ProtectionInfoKVOC64> entries_;

                    size_t GetBytesPerKey() const { return 8; }
                };
                class Handler {
                    public:
                        virtual ~Handler();
                        // 此类中的所有处理程序函数都提供默认实现，因此
                        // 在添加新成员函数时，我们不会在源代码级别破坏 Handler 的现有客户端。

                        // 默认实现将只调用不带列族的 Put 以实现向后兼容性。如果列族不是默认的，
                        // 该函数为 noop
                        // 如果启用了用户定义的时间戳，则“key”包含时间戳。

                };

            public:
                explicit WriteBatch(size_t reserved_bytes = 0, size_t max_bytes = 0)
                    : WriteBatch(reserved_bytes, max_bytes, 0, 0) {}
                
                // `protection_bytes_per_key` 是用于存储每个密钥条目的保护信息的字节数。当前支持的值为
                // 零（禁用）和八。
                explicit WriteBatch(size_t reserved_bytes, size_t max_bytes,
                                    size_t protection_bytes_per_key, size_t default_cf_ts_sz);
            
                
                std::unique_ptr<SavePoints> save_points_;

                // 当通过 WriteImpl 发送 WriteBatch 时，我们可能想要
                // 指定仅将批次的前 x 条记录写入
                // WAL。
                SavePoint wal_term_point_;

                // 批处理的内容是否是应用程序的最新状态，仅用于恢复？请参阅 TransactionOptions::use_only_the_last_commit_time_batch_for_recovery 以了解更多详细信息。
                bool is_latest_persistent_state_ = false;
                // 如果所有键都来自禁用用户定义
                // 时间戳或 UpdateTimestamps() 的列族，则为 False。
                // 如果上述 Put()、Delete()、
                // SingleDelete() 等 API 至少被调用一次，则此标志将设置为 true。
                // 调用 Put(ts)、Delete(ts)、SingleDelete(ts) 等不会将此标志
                // 设置为 true，因为假设这些 API 已经将
                // 时间戳设置为所需值。
                bool needs_in_place_update_ts_ = false;

                // 如果写入批次包含至少一个来自列族的键，则为 True
                // 启用用户定义的时间戳。
                bool has_key_with_ts_ = false;

                // 对于 HasXYZ。可变以允许惰性计算结果
                mutable std::atomic<uint32_t> content_flags_;
                uint32_t ComputeContentFlags() const;

                // rep_ 的最大尺寸。
                size_t max_bytes_;

                std::unique_ptr<ProtectionInfo> prot_info_;
                size_t default_cf_ts_sz_ = 0;

                bool track_timestamp_size_ = false;
                std::unordered_map<uint32_t, size_t> cf_id_to_ts_sz_;
            public:
                std::string rep_; // 请参阅 write_batch.cc 中的注释以了解 rep_ 的格式
        };
    } // namespace rocksdb
    
} // namespace latte

#endif