


#ifndef __LATTE_C_PLUS_ROCKSDB_WRITE_OPTIONS_H
#define __LATTE_C_PLUS_ROCKSDB_WRITE_OPTIONS_H
#include "env/env.h"
namespace latte
{
    
    namespace rocksdb
    {
        struct WriteOptions {
            Env::IOActivity io_activity = Env::IOActivity::kUnknown;

            WriteOptions() {}
            explicit WriteOptions(Env::IOActivity _io_activity);
        };

    } // namespace rocksdb
    

} // namespace latte




#endif

