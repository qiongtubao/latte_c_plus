


#include "gtest/gtest.h"
#include "db.h"

namespace latte {

    TEST(DBTest, Open) { 
        DB* db;
        OpenOptions op;
        op.create_if_missing = true;
        Status status = DB::Open(op, "/tmp/testdb", &db);
        assert(status.ok());
        delete(db);


    }

    TEST(DBTest, Get) {
        DB* db;
        ReadOptions ops;
        
    }
}