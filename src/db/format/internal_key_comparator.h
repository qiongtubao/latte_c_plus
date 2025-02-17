
#ifndef __LATTE_C_PLUS_INTERNAL_KEY_COMPARATOR_H
#define __LATTE_C_PLUS_INTERNAL_KEY_COMPARATOR_H

#include "comparator/comparator.h"
#include "internal_key.h"
namespace latte
{
    namespace leveldb
    {

        class InternalKeyComparator: public Comparator {
            private:
                const Comparator* user_comparator_;
            
        };
    } // namespace rocksdb

    namespace rocksdb
    {
        class InternalKeyComparator: public CompareInterface {
            public:
                // 使用默认构造函数构造的 `InternalKeyComparator` 不可使用
                // 并且在尝试使用它们进行比较时会出现段错误。
                InternalKeyComparator() = default;
                int Compare(const Slice& a, const Slice& b) const override;
                int Compare(const InternalKey& a, const InternalKey& b) const;
                // int Compare(const ParsedInternalKey& a, const ParsedInternalKey& b) const;
                // int Compare(const Slice& a, const ParsedInternalKey& b) const;
                // int Compare(const ParsedInternalKey& a, const Slice& b) const;

                // In this `Compare()` overload, the sequence numbers provided in
                // `a_global_seqno` and `b_global_seqno` override the sequence numbers in `a`
                // and `b`, respectively. To disable sequence number override(s), provide the
                // value `kDisableGlobalSequenceNumber`.
                int Compare(const Slice& a, SequenceNumber a_global_seqno, const Slice& b,
                            SequenceNumber b_global_seqno) const;
            private:
                const Comparator* user_comparator_;
            
        };
    } // namespace rocksdb
    
        
} // namespace latte

#endif