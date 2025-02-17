


#include "gtest/gtest.h"
#include "db.h"
#include "leveldb.h"
#include "rocksdb.h"

namespace latte {

    TEST(LevelDBTest, OpenEmpty) { 
        DB* db;
        OpenOptions op;
        op.create_if_missing = true;
        Status status = leveldb::LevelDB::Open(op, "/tmp/testdb", &db);
        assert(status.ok());
        delete(db);


    }

    TEST(RocksdbDBTest, OpenEmpty) {
        DB* db;
        OpenOptions op;
        op.create_if_missing = true;
        Status status = rocksdb::RocksDB::Open(op, "/tmp/testdb", &db);
        assert(status.ok());
        delete(db);

        
    }
}