



#ifndef __LATTE_C_PLUS_COLUMN_FAMILY_OPTIONS
#define __LATTE_C_PLUS_COLUMN_FAMILY_OPTIONS

#include <stddef.h>
#include <stdint.h>
#include "./advanced_column_family_options.h"
#include <vector>
#include "../shared/db_path.h"
#include "../options/open_options.h"
namespace latte
{
    namespace rocksdb
    {
        class ColumnFamilyOptions : public AdvancedColumnFamilyOptions {
            public:
                ColumnFamilyOptions();
                // Create ColumnFamilyOptions from Options
                explicit ColumnFamilyOptions(const RocksdbOpenOptions& options);
            
            public:
                size_t write_buffer_size = 64 << 20;
                std::vector<DbPath> cf_paths;

        };

        ColumnFamilyOptions SanitizeOptions(const ImmutableDBOptions& db_options,
                                    const ColumnFamilyOptions& src);
    } // namespace rocksdb
    
} // namespace latte


#endif