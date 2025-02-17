#ifndef __LATTE_C_PLUS_IMMUTABLE_OPTIONS_H
#define __LATTE_C_PLUS_IMMUTABLE_OPTIONS_H


#include "./immutable_db_options.h"
#include "./immutable_cf_options.h"


namespace latte
{
    namespace rocksdb
    {
        struct ImmutableOptions: public ImmutableDBOptions, public ImmutableCFOptions {
            explicit ImmutableOptions();
            explicit ImmutableOptions(const Options& options);

        };
    } // namespace rocksdb
    
} // namespace latte




#endif