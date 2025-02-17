



#ifndef __LATTE_C_PLUS_IMMUTSBLE_MEM_TABLE_OPTIONS_H
#define __LATTE_C_PLUS_IMMUTSBLE_MEM_TABLE_OPTIONS_H


#include "../options/immutable_options.h"
#include "../options/mutable_cf_options.h"
#include "../statistics/statistics.h"

namespace latte
{
    namespace rocksdb
    {
        struct ImmutableMemTableOptions {
            explicit ImmutableMemTableOptions(const ImmutableOptions& ioptions,
                                    const MutableCFOptions& mutable_cf_options);
            size_t arena_block_size;
            uint32_t memtable_prefix_bloom_bits;
            size_t memtable_huge_page_size;
            bool memtable_whole_key_filtering;
            bool inplace_update_support;
            size_t inplace_update_num_locks;
            UpdateStatus (*inplace_callback)(char* existing_value,
                                            uint32_t* existing_value_size,
                                            Slice delta_value,
                                            std::string* merged_value);
            size_t max_successive_merges;
            bool strict_max_successive_merges;
            Statistics* statistics;
            MergeOperator* merge_operator;
            Logger* info_log;
            uint32_t protection_bytes_per_key;
            bool allow_data_in_errors;
            bool paranoid_memory_checks;
        };
    } // namespace rocksdb
    
} // namespace latte



#endif