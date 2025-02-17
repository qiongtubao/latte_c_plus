

#ifndef __LATTE_C_PLUS_PROTECTION_INFO_KVOS_H
#define __LATTE_C_PLUS_PROTECTION_INFO_KVOS_H

namespace latte
{
    namespace rocksdb
    {
        
        template <typename T>
        class ProtectionInfoKVOS {
            public:
                ProtectionInfoKVOS() = default;

                ProtectionInfoKVO<T> StripS(SequenceNumber sequence_number) const;
        };

    } // namespace rocksdb
    
} // namespace latte


#endif