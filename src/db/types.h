




#ifndef __LATTE_C_PLUS_TYPES_H
#define __LATTE_C_PLUS_TYPES_H
#include <stdint.h>

#include <memory>
#include <unordered_map>

namespace latte
{
    namespace rocksdb
    {
        // Define all public custom types here.

        using ColumnFamilyId = uint32_t;

        // Represents a sequence number in a WAL file.
        using SequenceNumber = uint64_t;

        enum class WriteStallCondition {
            kDelayed,
            kStopped,
            // Always add new WriteStallCondition before `kNormal`
            kNormal,
        };
       
    } // namespace rocksdb
    
} // namespace latte


#endif