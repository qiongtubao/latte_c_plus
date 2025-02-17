


#ifndef __LATTE_C_PLUS_VERSION_H
#define __LATTE_C_PLUS_VERSION_H

#include "version_storage_info.h"
#include "version_set.h"
#include "version_edit.h"


namespace latte
{
    namespace rocksdb
    {
        class Version {
            public:
                VersionStorageInfo* storage_info() { return &storage_info_; }
            private:
                VersionStorageInfo storage_info_;
        };
    } // namespace name
    
} // namespace latte



#endif