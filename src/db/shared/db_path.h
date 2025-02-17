




#ifndef __LATTE_C_PLUS_DB_PATH_H
#define __LATTE_C_PLUS_DB_PATH_H
#include <string>

namespace latte
{
    namespace rocksdb
    {
        struct DbPath {
            std::string path;
            uint64_t target_size;
            DbPath() : target_size(0) {}
            DbPath(const std::string& p, uint64_t t) : path(p), target_size(t) {}
        };
    } // namespace rocksdb
    
} // namespace latte


#endif

        