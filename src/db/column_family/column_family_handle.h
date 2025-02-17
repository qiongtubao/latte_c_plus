


#ifndef __LATTE_C_PLUS_COLUMN_FAMILY_HANDLE
#define __LATTE_C_PLUS_COLUMN_FAMILY_HANDLE



#include "./column_family_mem_tables.h"

namespace latte
{
    namespace rocksdb
    {
        class RocksDB;
        class ColumnFamilyHandle {
            public:
                virtual ~ColumnFamilyHandle() {}

        };
        class ColumnFamilyHandleImpl: public ColumnFamilyHandle {
            public:
                // create while holding the mutex
                ColumnFamilyHandleImpl(ColumnFamilyData* cfd, RocksDB* db,
                                        InstrumentedMutex* mutex);
                // destroy without mutex
                ~ColumnFamilyHandleImpl(); 

                ColumnFamilyData* cfd() const { return cfd_; }
                RocksDB* db() const { return db_; }
            private:
                ColumnFamilyData* cfd_;
                RocksDB* db_;
                InstrumentedMutex* mutex_;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif