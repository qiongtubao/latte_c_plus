
#ifndef __LATTE_C_PLUS_IO_FILE_UTIL_H
#define __LATTE_C_PLUS_IO_FILE_UTIL_H


#include "io/file_system.h"
namespace latte
{
    namespace rocksdb
    {
        inline bool CheckFSFeatureSupport(FileSystem* fs, FSSupportedOps feat) {
            int64_t supported_ops = 0;
            fs->SupportedOps(supported_ops);
            if (supported_ops & (1ULL << feat)) {
                return true;
            }
            return false;
        }
    } // namespace name
    
} // namespace latte


#endif