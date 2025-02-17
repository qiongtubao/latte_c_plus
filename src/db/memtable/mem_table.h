



#ifndef __LATTE_C_PLUS_MEM_TABLE_H
#define __LATTE_C_PLUS_MEM_TABLE_H

#include "./read_only_mem_table.h"
#include "./mem_table_rep.h"
#include "../format/internal_key_comparator.h"
#include "../types.h"
#include <vector>
#include "system/system_clock.h"
#include "port/port_posix.h"
#include "../slice_transform.h"
#include "./immutable_mem_table_options.h"

namespace latte
{
    namespace rocksdb
    {
        class MemTable final : public ReadOnlyMemTable {
            public:
                struct KeyComparator final : public MemTableRep::KeyComparator {
                    const InternalKeyComparator comparator;
                    explicit KeyComparator(const InternalKeyComparator& c) : comparator(c) {}
                    int operator()(const char* prefix_len_key1,
                                const char* prefix_len_key2) const override;
                    int operator()(const char* prefix_len_key,
                                const DecodedType& key) const override;
                }; 

                // return true if the current MemTableRep supports merge operator.
                bool IsMergeOperatorSupported() const {
                    return table_->IsMergeOperatorSupported();
                }

                // return true if the current MemTableRep supports snapshots.
                // inplace update prevents snapshots,
                bool IsSnapshotSupported() const {
                    return table_->IsSnapshotSupported() && !moptions_.inplace_update_support;
                }



            public:
                enum FlushStateEnum { FLUSH_NOT_REQUESTED, FLUSH_REQUESTED, FLUSH_SCHEDULED };

                friend class MemTableIterator;
                friend class MemTableBackwardIterator;
                friend class MemTableList;

                KeyComparator comparator_;
                const ImmutableMemTableOptions moptions_;
                const size_t kArenaBlockSize;
                // AllocTracker mem_tracker_;
                // ConcurrentArena arena_;
                std::unique_ptr<MemTableRep> table_;
                std::unique_ptr<MemTableRep> range_del_table_;
                std::atomic_bool is_range_del_table_empty_;

                // Total data size of all data inserted
                std::atomic<uint64_t> data_size_;
                std::atomic<uint64_t> num_entries_;
                std::atomic<uint64_t> num_deletes_;
                std::atomic<uint64_t> num_range_deletes_;

                // Dynamically changeable memtable option
                std::atomic<size_t> write_buffer_size_;

                // The sequence number of the kv that was inserted first
                std::atomic<SequenceNumber> first_seqno_;

                // The db sequence number at the time of creation or kMaxSequenceNumber
                // if not set.
                std::atomic<SequenceNumber> earliest_seqno_;

                SequenceNumber creation_seq_;

                // the earliest log containing a prepared section
                // which has been inserted into this memtable.
                std::atomic<uint64_t> min_prep_log_referenced_;

                // rw locks for inplace updates
                std::vector<port::RWMutex> locks_;

                const SliceTransform* const prefix_extractor_;
                // std::unique_ptr<DynamicBloom> bloom_filter_;

                std::atomic<FlushStateEnum> flush_state_;

                SystemClock* clock_;

                // Extract sequential insert prefixes.
                const SliceTransform* insert_with_hint_prefix_extractor_;

                // Insert hints for each prefix.
                // UnorderedMapH<Slice, void*, SliceHasher32> insert_hints_;

                // Timestamp of oldest key
                std::atomic<uint64_t> oldest_key_time_;

                // keep track of memory usage in table_, arena_, and range_del_table_.
                // Gets refreshed inside `ApproximateMemoryUsage()` or `ShouldFlushNow`
                std::atomic<uint64_t> approximate_memory_usage_;

                // max range deletions in a memtable,  before automatic flushing, 0 for
                // unlimited.
                uint32_t memtable_max_range_deletions_ = 0;

                // Size in bytes for the user-defined timestamps.
                size_t ts_sz_;

                // Whether to persist user-defined timestamps
                bool persist_user_defined_timestamps_;

                // Newest user-defined timestamp contained in this MemTable. For ts1, and ts2
                // if Comparator::CompareTimestamp(ts1, ts2) > 0, ts1 is considered newer than
                // ts2. We track this field for a MemTable if its column family has UDT
                // feature enabled and the `persist_user_defined_timestamp` flag is false.
                // Otherwise, this field just contains an empty Slice.
                Slice newest_udt_;
        };
    } // namespace rocksdb
    
} // namespace latte



#endif