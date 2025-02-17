
#ifndef __LATTE_C_PLUS_IMMUTABLE_CF_OPTIONS_H
#define __LATTE_C_PLUS_IMMUTABLE_CF_OPTIONS_H

#include <string>
#include "advanced_options.h"
#include "io/types.h"
#include "../compaction/compaction_filter.h"
#include "./merge_operator.h"
#include "../compaction/compaction_filter_factory.h"
#include "memtable/mem_table_rep_factory.h"
namespace latte
{
    
    namespace rocksdb
    {
        struct ImmutableCFOptions {
            public:
                explicit ImmutableCFOptions();
                CompactionStyle compaction_style;

                // CompactionPri compaction_pri;

                const Comparator* user_comparator;
                InternalKeyComparator internal_comparator;  // Only in Immutable

                std::shared_ptr<MergeOperator> merge_operator;

                const CompactionFilter* compaction_filter;

                std::shared_ptr<CompactionFilterFactory> compaction_filter_factory;

                int min_write_buffer_number_to_merge;

                int max_write_buffer_number_to_maintain;

                int64_t max_write_buffer_size_to_maintain;

                bool inplace_update_support;

                // UpdateStatus (*inplace_callback)(char* existing_value,
                //                                 uint32_t* existing_value_size,
                //                                 Slice delta_value,
                //                                 std::string* merged_value);

                std::shared_ptr<MemTableRepFactory> memtable_factory;

                // Options::TablePropertiesCollectorFactories
                //     table_properties_collector_factories;

                // This options is required by PlainTableReader. May need to move it
                // to PlainTableOptions just like bloom_bits_per_key
                uint32_t bloom_locality;

                bool level_compaction_dynamic_level_bytes;

                int num_levels;

                bool optimize_filters_for_hits;

                bool force_consistency_checks;

                Temperature default_temperature;

                std::shared_ptr<const SliceTransform>
                    memtable_insert_with_hint_prefix_extractor;

                std::vector<DbPath> cf_paths;

                // std::shared_ptr<ConcurrentTaskLimiter> compaction_thread_limiter;

                // std::shared_ptr<SstPartitionerFactory> sst_partitioner_factory;

                // std::shared_ptr<Cache> blob_cache;

                bool persist_user_defined_timestamps;
        };
    } // namespace rocksdb
    
} // namespace latte




#endif