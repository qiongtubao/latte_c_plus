
#ifndef __LATTE_C_PLUS_ROCKSDB_OPTIONS_HELPER_H
#define __LATTE_C_PLUS_ROCKSDB_OPTIONS_HELPER_H

#include "status/status.h"
#include "open_options.h"
#include "../column_family/column_family_options.h"

namespace latte
{
    namespace rocksdb
    {
        // 检查 DBOptions 和 ColumnFamilyOptions 的组合是否有效
        Status ValidateOptions(const DBOptions& db_opts,
                       const ColumnFamilyOptions& cf_opts);
    } // namespace rocksdb
    
} // namespace latte


#endif