

#ifndef __LATTE_C_PLUS_CACHE_ENTRY_ROLE_H
#define __LATTE_C_PLUS_CACHE_ENTRY_ROLE_H


namespace latte
{
    namespace rocksdb
    {
        // Classifications of block cache entries.
        //
        // Developer notes: Adding a new enum to this class requires corresponding
        // updates to `kCacheEntryRoleToCamelString` and
        // `kCacheEntryRoleToHyphenString`. Do not add to this enum after `kMisc` since
        // `kNumCacheEntryRoles` assumes `kMisc` comes last.
        enum class CacheEntryRole {
            // Block-based table data block
            kDataBlock,
            // Block-based table filter block (full or partitioned)
            kFilterBlock,
            // Block-based table metadata block for partitioned filter
            kFilterMetaBlock,
            // OBSOLETE / DEPRECATED: old/removed block-based filter
            kDeprecatedFilterBlock,
            // Block-based table index block
            kIndexBlock,
            // Other kinds of block-based table block
            kOtherBlock,
            // WriteBufferManager's charge to account for its memtable usage
            kWriteBuffer,
            // Compression dictionary building buffer's charge to account for
            // its memory usage
            kCompressionDictionaryBuildingBuffer,
            // Filter's charge to account for
            // (new) bloom and ribbon filter construction's memory usage
            kFilterConstruction,
            // BlockBasedTableReader's charge to account for its memory usage
            kBlockBasedTableReader,
            // FileMetadata's charge to account for its memory usage
            kFileMetadata,
            // Blob value (when using the same cache as block cache and blob cache)
            kBlobValue,
            // Blob cache's charge to account for its memory usage (when using a
            // separate block cache and blob cache)
            kBlobCache,
            // Default bucket, for miscellaneous cache entries. Do not use for
            // entries that could potentially add up to large usage.
            kMisc,
        };
    } // namespace rocksdb
    
} // namespace latte



#endif 