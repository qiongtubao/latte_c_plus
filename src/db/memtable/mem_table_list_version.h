




#ifndef __LATTE_C_PLUS_MEM_TABLE_LIST_VERSION_H
#define __LATTE_C_PLUS_MEM_TABLE_LIST_VERSION_H

#include <string>

namespace latte
{
    namespace rocksdb
    {
        class MemTableListVersion {
            public:
                explicit MemTableListVersion(size_t* parent_memtable_list_memory_usage,
                               const MemTableListVersion& old);
                
        };
    } // namespace rocksdb
    
} // namespace latte

#endif