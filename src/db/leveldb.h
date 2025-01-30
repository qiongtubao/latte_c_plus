

#include "db.h"

namespace latte
{
    namespace leveldb
    {
        class MemTable;
        class TableCache;
        class Version;
        class VersionEdit;
        class VersionSet;

         class LevelDB: public DB {
            public:
                static Status Open(const OpenOptions& options, const std::string& name, DB** dbptr);

                
        };

    } // namespace leveldb
    
    
} // namespace latte
