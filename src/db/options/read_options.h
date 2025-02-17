


#ifndef __LATTE_C_PLUS_ROCKSDB_READ_OPTIONS_H
#define __LATTE_C_PLUS_ROCKSDB_READ_OPTIONS_H
#include "env/env.h"
namespace latte
{
    namespace rocksdb {

        struct ReadOptions {
            // EXPERIMENTAL
            Env::IOActivity io_activity = Env::IOActivity::kUnknown;

            // *** END options for RocksDB internal use only ***

            ReadOptions() {}
            ReadOptions(bool _verify_checksums, bool _fill_cache);
            explicit ReadOptions(Env::IOActivity _io_activity);
        };
    }
} // namespace latte

#endif
