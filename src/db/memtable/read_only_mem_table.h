


#ifndef __LATTE_C_PLUS_READ_ONLY_MEM_TABLE_H
#define __LATTE_C_PLUS_READ_ONLY_MEM_TABLE_H

namespace latte
{
    namespace rocksdb
    {
        class ReadOnlyMemTable {
            public:
                // Do not delete this MemTable unless Unref() indicates it not in use.
                virtual ~ReadOnlyMemTable() = default;
                
        };
    } // namespace rocksdb
    
} // namespace latte

#endif