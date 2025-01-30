



#include "db.h"

namespace latte
{
    namespace rocksdb
    {
        class Arena;
        class ArenaWrappedDbIter;
        class InMemoryStatsHistoryIterator;
        class MemTable;
        class PersistentStatsHistoryIterator;
        class TableCache;
        class TaskLimiterToken;
        class Version;
        class VersionEdit;
        class VersionSet;
        class WriteCallback;
        struct JobContext;
        struct ExternalSstFileInfo;
        struct MemTableInfo;

        class RocksDB : public DB {
            public:
                static Status Open(const OpenOptions& options, const std::string& name, DB** dbptr);
        };

        
    } // namespace rocksdb
    
} // namespace latte
