



#ifndef __LATTE_C_PLUS_WRITE_BATCH_BASE_H
#define __LATTE_C_PLUS_WRITE_BATCH_BASE_H

namespace latte
{
    namespace leveldb
    {
        class WriteBatch {

        };
    } // namespace leveldb

    namespace rocksdb
    {
        class WriteBatchBase {
            public:
                virtual ~WriteBatchBase() {}
        };
    } // namespace rocksdb
    
} // namespace latte

#endif