
#include "advanced_column_family_options.h"

namespace latte
{


    struct OpenOptions {
        public:
            bool create_if_missing;

    };

    namespace rocksdb
    {
        struct DBOptions {

        };

        struct ColumnFamilyOptions: public AdvancedColumnFamilyOptions {

        };

        struct RocksdbOpenOptions: public OpenOptions, public DBOptions, public ColumnFamilyOptions  {

        };

    } // namespace rocksdb
    

    
    
} // namespace latte

