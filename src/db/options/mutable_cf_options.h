


#ifndef __LATTE_C_PLUS_MUTABLE_CF_OPTIONS_H
#define __LATTE_C_PLUS_MUTABLE_CF_OPTIONS_H

#include <cinttypes>
#include "../slice_transform.h" 

namespace latte
{
    namespace rocksdb
    {
        struct MutableCFOptions {
            // Memtable related options
            size_t write_buffer_size;
            int max_write_buffer_number;
            size_t arena_block_size;
            double memtable_prefix_bloom_size_ratio;
            bool memtable_whole_key_filtering;
            size_t memtable_huge_page_size;
            size_t max_successive_merges;
            bool strict_max_successive_merges;
            size_t inplace_update_num_locks;
            // NOTE: if too many shared_ptr make their way into MutableCFOptions, the
            // copy performance might suffer enough to warrant aggregating them in an
            // immutable+copy-on-write sub-object managed through a single shared_ptr.
            std::shared_ptr<const SliceTransform> prefix_extractor;
            // [experimental]
            // Used to activate or deactive the Mempurge feature (memtable garbage
            // collection). (deactivated by default). At every flush, the total useful
            // payload (total entries minus garbage entries) is estimated as a ratio
            // [useful payload bytes]/[size of a memtable (in bytes)]. This ratio is then
            // compared to this `threshold` value:
            //     - if ratio<threshold: the flush is replaced by a mempurge operation
            //     - else: a regular flush operation takes place.
            // Threshold values:
            //   0.0: mempurge deactivated (default).
            //   1.0: recommended threshold value.
            //   >1.0 : aggressive mempurge.
            //   0 < threshold < 1.0: mempurge triggered only for very low useful payload
            //   ratios.
            // [experimental]
            double experimental_mempurge_threshold;

            // Compaction related options
            bool disable_auto_compactions;
            std::shared_ptr<TableFactory> table_factory;
            uint64_t soft_pending_compaction_bytes_limit;
            uint64_t hard_pending_compaction_bytes_limit;
            int level0_file_num_compaction_trigger;
            int level0_slowdown_writes_trigger;
            int level0_stop_writes_trigger;
            uint64_t max_compaction_bytes;
            uint64_t target_file_size_base;
            int target_file_size_multiplier;
            uint64_t max_bytes_for_level_base;
            double max_bytes_for_level_multiplier;
            uint64_t ttl;
            uint64_t periodic_compaction_seconds;
            std::vector<int> max_bytes_for_level_multiplier_additional;
            CompactionOptionsFIFO compaction_options_fifo;
            CompactionOptionsUniversal compaction_options_universal;
            uint64_t preclude_last_level_data_seconds;
            uint64_t preserve_internal_time_seconds;

            // Blob file related options
            bool enable_blob_files;
            uint64_t min_blob_size;
            uint64_t blob_file_size;
            CompressionType blob_compression_type;
            bool enable_blob_garbage_collection;
            double blob_garbage_collection_age_cutoff;
            double blob_garbage_collection_force_threshold;
            uint64_t blob_compaction_readahead_size;
            int blob_file_starting_level;
            PrepopulateBlobCache prepopulate_blob_cache;

            // Misc options
            uint64_t max_sequential_skip_in_iterations;
            bool paranoid_file_checks;
            bool report_bg_io_stats;
            CompressionType compression;
            CompressionType bottommost_compression;
            CompressionOptions compression_opts;
            CompressionOptions bottommost_compression_opts;
            Temperature last_level_temperature;
            Temperature default_write_temperature;
            uint32_t memtable_protection_bytes_per_key;
            uint8_t block_protection_bytes_per_key;
            bool paranoid_memory_checks;

            uint64_t sample_for_compression;
            std::vector<CompressionType> compression_per_level;
            uint32_t memtable_max_range_deletions;
            uint32_t bottommost_file_compaction_delay;
            uint32_t uncache_aggressiveness;

            // Derived options
            // Per-level target file size.
            std::vector<uint64_t> max_file_size;
        };
    } // namespace rocksdb
    
} // namespace latte





#endif