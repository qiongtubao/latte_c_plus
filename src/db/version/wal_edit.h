



#ifndef __LATTE_C_PLUS_WAL_EDIT_H
#define __LATTE_C_PLUS_WAL_EDIT_H

#include "status.h"
#include "../env/env.h"
#include <string>

namespace latte
{
    namespace rocksdb
    {
        using WalNumber = uint64_t;
        class WalMetadata {
            public:
                WalMetadata() = default;    
        };

        class WalSet {
            public:
                const std::map<WalNumber, WalMetadata>& GetWals() const { return wals_; }
                Status CheckWals(
                    Env* env,
                    const std::unordered_map<WalNumber, std::string>& logs_on_disk) const;
            private:
                std::map<WalNumber, WalMetadata>wals_;
                WalNumber min_wal_number_to_keep_ = 0;
        };
    } // namespace rocksdb
    
} // namespace latte




#endif
