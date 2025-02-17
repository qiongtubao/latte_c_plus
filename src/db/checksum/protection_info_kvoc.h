




#ifndef __LATTE_C_PLUS_PROTECTION_INFO_KVOC_H
#define __LATTE_C_PLUS_PROTECTION_INFO_KVOC_H


namespace latte
{
    namespace rocksdb
    {
        
        template <typename T>
        class ProtectionInfoKVOC {
            public:
                ProtectionInfoKVOC() = default;

                ProtectionInfoKVO<T> StripC(ColumnFamilyId column_family_id) const;
        };

    } // namespace rocksdb
    
} // namespace latte


#endif