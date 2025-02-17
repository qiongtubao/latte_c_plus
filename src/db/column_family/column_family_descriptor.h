



#ifndef __LATTE_C_PLUS_COLUMN_FAMILY_DESCRIPTOR_H
#define __LATTE_C_PLUS_COLUMN_FAMILY_DESCRIPTOR_H

#include <string>
#include "./column_family_options.h"

namespace latte
{
    namespace rocksdb
    {
        const std::string kDefaultColumnFamilyName("default");
        struct ColumnFamilyDescriptor {
            std::string name;
            ColumnFamilyOptions options;
            // 使用与此句柄关联的列族的最新描述符填充“*desc”。
            // 由于它使用最新信息填充“*desc”，
            // 因此此调用可能会在内部锁定并释放 DB 
            // 互斥锁以访问最新的 CF 选项。
            // 此外，所有指针类型的选项都不能比原始选项存在的时间更长地被引用。
            //
            // 请注意，RocksDBLite 不支持此功能。
            ColumnFamilyDescriptor()
                : name(kDefaultColumnFamilyName), options(ColumnFamilyOptions()) {}
            ColumnFamilyDescriptor(const std::string& _name,
                         const ColumnFamilyOptions& _options)
                : name(_name), options(_options) {}
        };

        
    } // namespace rocksdb
    
} // namespace latte


#endif