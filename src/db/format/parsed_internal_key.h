

#ifndef __LATTE_C_PLUS_ITER_KEY_H
#define __LATTE_C_PLUS_ITER_KEY_H


#include "slice/slice.h"
#include "../types.h"
#include "../format/config.h"
#include "../comparator/comparator.h"

namespace latte
{
    namespace rocksdb
    {
        // The data structure that represents an internal key in the way that user_key,
        // sequence number and type are stored in separated forms.
        struct ParsedInternalKey {
            Slice user_key;
            SequenceNumber sequence;
            ValueType type;

            ParsedInternalKey()
                : sequence(kMaxSequenceNumber),
                    type(kTypeDeletion)  // Make code analyzer happy
            {}                         // Intentionally left uninitialized (for speed)
            // u contains timestamp if user timestamp feature is enabled.
            ParsedInternalKey(const Slice& u, const SequenceNumber& seq, ValueType t)
                : user_key(u), sequence(seq), type(t) {}
            std::string DebugString(bool log_err_key, bool hex,
                                    const Comparator* ucmp = nullptr) const;

            void clear() {
                user_key.clear();
                sequence = 0;
                type = kTypeDeletion;
            }

            void SetTimestamp(const Slice& ts) {
                assert(ts.size() <= user_key.size());
                const char* addr = user_key.data() + user_key.size() - ts.size();
                memcpy(const_cast<char*>(addr), ts.data(), ts.size());
            }

            Slice GetTimestamp(size_t ts_sz) {
                assert(ts_sz <= user_key.size());
                const char* addr = user_key.data() + user_key.size() - ts_sz;
                return Slice(const_cast<char*>(addr), ts_sz);
            }
        };
    } // namespace rocksdb
    
} // namespace latte




#endif