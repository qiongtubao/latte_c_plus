



#ifndef __LATTE_C_PLUS_MEM_TABLE_REP_H
#define __LATTE_C_PLUS_MEM_TABLE_REP_H

#include "./read_only_mem_table.h"
#include "slice/slice.h"
namespace latte
{
    namespace rocksdb
    {
        Slice GetLengthPrefixedSlice(const char* data);
        class MemTableRep {
            public:
                class KeyComparator {
                    public:
                        using DecodedType = Slice;
                        virtual DecodedType decode_key(const char* key) const {
                            // key 的格式已固定，可视为 API 契约的一部分。详情请参阅 MemTable::Add。
                            return GetLengthPrefixedSlice(key);
                        }

                        // Compare a and b. Return a negative value if a is less than b, 0 if they
                        // are equal, and a positive value if a is greater than b
                        virtual int operator()(const char* prefix_len_key1,
                                            const char* prefix_len_key2) const = 0;

                        virtual int operator()(const char* prefix_len_key,
                                            const Slice& key) const = 0;

                        virtual ~KeyComparator() {}

                        
                };
            public:

                // Return true if the current MemTableRep supports merge operator.
                // Default: true
                virtual bool IsMergeOperatorSupported() const { return true; }

                // Return true if the current MemTableRep supports snapshot
                // Default: true
                virtual bool IsSnapshotSupported() const { return true; }

            protected:
                // When *key is an internal key concatenated with the value, returns the
                // user key.
                virtual Slice UserKey(const char* key) const;

                // Allocator* allocator_;
        };
    } // namespace rocksdb
    
} // namespace latte



#endif