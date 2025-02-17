


#ifndef __LATTE_C_PLUS_PROTECTION_INFO_H
#define __LATTE_C_PLUS_PROTECTION_INFO_H

#include "protection_info_kvo.h"
#include "protection_info_kvoc.h"
#include "protection_info_kvos.h"
#include <cinttypes>

namespace latte
{
    namespace rocksdb
    {
        // Aliases for 64-bit protection infos.
        using ProtectionInfo64 = ProtectionInfo<uint64_t>;
        using ProtectionInfoKVO64 = ProtectionInfoKVO<uint64_t>;
        using ProtectionInfoKVOC64 = ProtectionInfoKVOC<uint64_t>;
        using ProtectionInfoKVOS64 = ProtectionInfoKVOS<uint64_t>;

        template <typename T>
        class ProtectionInfo {
            public:
                ProtectionInfo() = default;

                Status GetStatus() const;
                ProtectionInfo(T val) : val_(val) {
                    static_assert(sizeof(ProtectionInfo<T>) == sizeof(T), "");
                }
        };

    } // namespace rocksdb
    
} // namespace latte


#endif