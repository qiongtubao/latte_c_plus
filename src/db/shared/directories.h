





#ifndef __LATTE_C_PLUS_ENV_DIRECTORIES_H
#define __LATTE_C_PLUS_ENV_DIRECTORIES_H

#include "io/io_status.h"
#include "db_path.h"
#include "io/file_system.h"
namespace latte
{
    namespace rocksdb
    {
        class Directories {
            public:
                IOStatus SetDirectories(FileSystem* fs, const std::string& dbname,
                                        const std::string& wal_dir,
                                        const std::vector<DbPath>& data_paths);

                FSDirectory* GetDbDir() { return db_dir_.get(); }
            private:
                std::unique_ptr<FSDirectory> db_dir_;
                std::vector<std::unique_ptr<FSDirectory>> data_dirs_;
                std::unique_ptr<FSDirectory> wal_dir_;
        };
    } // namespace rocksdb
    

    
} // namespace latte
#endif
