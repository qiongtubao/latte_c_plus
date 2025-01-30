
#include "leveldb.h"
namespace latte
{   
    namespace leveldb
    {
        Status LevelDB::Open(const OpenOptions& options, const std::string& name, DB** dbptr) {
            return Status::OK();
        };

    } // namespace leveldb
    
    
} // namespace latte
