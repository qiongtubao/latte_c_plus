
#include "../format/internal_key.h"
namespace latte
{
    namespace leveldb
    {
        struct ManualCompaction {
            int level;
            bool done;
            const InternalKey* begin;
            const InternalKey* end;
            InternalKey tmp_storage;
        };
    } // namespace level
    
} // namespace latte


