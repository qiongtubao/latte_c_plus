


#ifndef __LATTE_C_PLUS_TABLE_PROPERTIES_H
#define __LATTE_C_PLUS_TABLE_PROPERTIES_H

#include <cstdint>
#include <string>
#include <map>
#include "status/status.h"
#include "io/file_system.h"

namespace latte
{   
    namespace rocksdb
    {
        using UserCollectedProperties = std::map<std::string, std::string>;
        // TableProperties contains a bunch of read-only properties of its associated
        // table.
        struct TableProperties {
            public:
                // the file number at creation time, or 0 for unknown. When known,
                // combining with db_session_id must uniquely identify an SST file.
                uint64_t orig_file_number = 0;
                // the total size of all data blocks.
                uint64_t data_size = 0;
                // the size of index block.
                uint64_t index_size = 0;
                // Total number of index partitions if kTwoLevelIndexSearch is used
                uint64_t index_partitions = 0;
                // Size of the top-level index if kTwoLevelIndexSearch is used
                uint64_t top_level_index_size = 0;
                // Whether the index key is user key. Otherwise it includes 8 byte of sequence
                // number added by internal key format.
                uint64_t index_key_is_user_key = 0;
                // Whether delta encoding is used to encode the index values.
                uint64_t index_value_is_delta_encoded = 0;
                // the size of filter block.
                uint64_t filter_size = 0;
                // total raw (uncompressed, undelineated) key size
                uint64_t raw_key_size = 0;
                // total raw (uncompressed, undelineated) value size
                uint64_t raw_value_size = 0;
                // the number of blocks in this table
                uint64_t num_data_blocks = 0;
                // the number of entries in this table
                uint64_t num_entries = 0;
                // the number of unique entries (keys or prefixes) added to filters
                uint64_t num_filter_entries = 0;
                // the number of deletions in the table
                uint64_t num_deletions = 0;
                // the number of merge operands in the table
                uint64_t num_merge_operands = 0;
                // the number of range deletions in this table
                uint64_t num_range_deletions = 0;
                // format version, reserved for backward compatibility
                uint64_t format_version = 0;
                // If 0, key is variable length. Otherwise number of bytes for each key.
                uint64_t fixed_key_len = 0;
                // ID of column family for this SST file, corresponding to the CF identified
                // by column_family_name.
                uint64_t column_family_id = latte::
                    TablePropertiesCollectorFactory::Context::kUnknownColumnFamily;

                // Oldest ancester time. 0 means unknown.
                //
                // For flush output file, oldest ancestor time is the oldest key time in the
                // file.  If the oldest key time is not available, flush time is used.
                //
                // For compaction output file, oldest ancestor time is the oldest
                // among all the oldest key time of its input files, since the file could be
                // the compaction output from other SST files, which could in turn be outputs
                // for compact older SST files. If that's not available, creation time of this
                // compaction output file is used.
                //
                // TODO(sagar0): Should be changed to oldest_ancester_time ... but don't know
                // the full implications of backward compatibility. Hence retaining for now.
                uint64_t creation_time = 0;

                // Timestamp of the earliest key. 0 means unknown.
                uint64_t oldest_key_time = 0;
                // Timestamp of the newest key. 0 means unknown.
                uint64_t newest_key_time = 0;
                // Actual SST file creation time. 0 means unknown.
                uint64_t file_creation_time = 0;
                // Estimated size of data blocks if compressed using a relatively slower
                // compression algorithm (see `ColumnFamilyOptions::sample_for_compression`).
                // 0 means unknown.
                uint64_t slow_compression_estimated_data_size = 0;
                // Estimated size of data blocks if compressed using a relatively faster
                // compression algorithm (see `ColumnFamilyOptions::sample_for_compression`).
                // 0 means unknown.
                uint64_t fast_compression_estimated_data_size = 0;
                // Offset of the value of the property "external sst file global seqno" in the
                // file if the property exists.
                // 0 means not exists.
                uint64_t external_sst_file_global_seqno_offset = 0;

                // Offset where the "tail" part of SST file starts
                // "Tail" refers to all blocks after data blocks till the end of the SST file
                uint64_t tail_start_offset = 0;

                // Value of the `AdvancedColumnFamilyOptions.persist_user_defined_timestamps`
                // when the file is created. Default to be true, only when this flag is false,
                // it's explicitly written to meta properties block.
                uint64_t user_defined_timestamps_persisted = 1;

                // The largest sequence number of keys in this file.
                // UINT64_MAX means unknown.
                // Only written to properties block if known (should be known unless the
                // table is empty).
                uint64_t key_largest_seqno = UINT64_MAX;

                // DB identity
                // db_id is an identifier generated the first time the DB is created
                // If DB identity is unset or unassigned, `db_id` will be an empty string.
                std::string db_id;

                // DB session identity
                // db_session_id is an identifier that gets reset every time the DB is opened
                // If DB session identity is unset or unassigned, `db_session_id` will be an
                // empty string.
                std::string db_session_id;

                // Location of the machine hosting the DB instance
                // db_host_id identifies the location of the host in some form
                // (hostname by default, but can also be any string of the user's choosing).
                // It can potentially change whenever the DB is opened
                std::string db_host_id;

                // Name of the column family with which this SST file is associated.
                // If column family is unknown, `column_family_name` will be an empty string.
                std::string column_family_name;

                // The name of the filter policy used in this table.
                // If no filter policy is used, `filter_policy_name` will be an empty string.
                std::string filter_policy_name;

                // The name of the comparator used in this table.
                std::string comparator_name;

                // The name of the merge operator used in this table.
                // If no merge operator is used, `merge_operator_name` will be "nullptr".
                std::string merge_operator_name;

                // The name of the prefix extractor used in this table
                // If no prefix extractor is used, `prefix_extractor_name` will be "nullptr".
                std::string prefix_extractor_name;

                // The names of the property collectors factories used in this table
                // separated by commas
                // {collector_name[1]},{collector_name[2]},{collector_name[3]} ..
                std::string property_collectors_names;

                // The compression algo used to compress the SST files.
                std::string compression_name;

                // Compression options used to compress the SST files.
                std::string compression_options;

                // Sequence number to time mapping, delta encoded.
                std::string seqno_to_time_mapping;

                // user collected properties
                UserCollectedProperties user_collected_properties;
                UserCollectedProperties readable_properties;

                // convert this object to a human readable form
                //   @prop_delim: delimiter for each property.
                std::string ToString(const std::string& prop_delim = "; ",
                                    const std::string& kv_delim = "=") const;

                // Aggregate the numerical member variables of the specified
                // TableProperties.
                void Add(const TableProperties& tp);

                // Subset of properties that make sense when added together
                // between tables. Keys match field names in this class instead
                // of using full property names.
                std::map<std::string, uint64_t> GetAggregatablePropertiesAsMap() const;

                // Return the approximated memory usage of this TableProperties object,
                // including memory used by the string properties and UserCollectedProperties
                std::size_t ApproximateMemoryUsage() const;

                // Serialize and deserialize Table Properties
                Status Serialize(const ConfigOptions& opts, std::string* output) const;
                static Status Parse(const ConfigOptions& opts, const std::string& serialized,
                                    TableProperties* table_properties);
                bool AreEqual(const ConfigOptions& opts,
                                const TableProperties* other_table_properties,
                                std::string* mismatch) const;
                };
    } // namespace rocksdb
    
} // namespace latte


#endif