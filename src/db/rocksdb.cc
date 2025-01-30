


#include "rocksdb.h"

namespace latte
{
    namespace rocksdb
    {
        Status RocksDB::Open(const OpenOptions& options, const std::string& name, DB** dbptr) {
            return Status::OK();
        };
    } // namespace rocksdb
    
} // namespace latte
