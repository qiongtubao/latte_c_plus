


#ifndef __LATTE_C_PLUS_SEQNO_TO_TIME_MAPPING_H
#define __LATTE_C_PLUS_SEQNO_TO_TIME_MAPPING_H


#include <string>
#include "status/status.h"
#include "../types.h"

namespace latte
{
    namespace rocksdb
    {
        class SeqnoToTimeMapping {
            public:
                struct SeqnoTimePair {
                    SequenceNumber seqno = 0;
                    uint64_t time = 0;

                    SeqnoTimePair() = default;
                    SeqnoTimePair(SequenceNumber _seqno, uint64_t _time)
                        : seqno(_seqno), time(_time) {}

                    // Encode to dest string
                    void Encode(std::string& dest) const;

                    // Decode the value from input Slice and remove it from the input
                    Status Decode(Slice& input);

                    // For delta encoding
                    SeqnoTimePair ComputeDelta(const SeqnoTimePair& base) const {
                        return {seqno - base.seqno, time - base.time};
                    }

                    // For delta decoding
                    void ApplyDelta(const SeqnoTimePair& delta_or_base) {
                        seqno += delta_or_base.seqno;
                        time += delta_or_base.time;
                    }

                    // If another pair can be combined into this one (for optimizing
                    // normal SeqnoToTimeMapping behavior), then this mapping is modified
                    // and true is returned, indicating the other mapping can be discarded.
                    // Otherwise false is returned and nothing is changed.
                    bool Merge(const SeqnoTimePair& other);

                    // Ordering used for Sort()
                    bool operator<(const SeqnoTimePair& other) const {
                        return std::tie(seqno, time) < std::tie(other.seqno, other.time);
                    }

                    bool operator==(const SeqnoTimePair& other) const {
                        return std::tie(seqno, time) == std::tie(other.seqno, other.time);
                    }

                    static bool SeqnoLess(const SeqnoTimePair& a, const SeqnoTimePair& b) {
                        return a.seqno < b.seqno;
                    }

                    static bool TimeLess(const SeqnoTimePair& a, const SeqnoTimePair& b) {
                        return a.time < b.time;
                    }
                };
        };
    } // namespace rocksdb
    
} // namespace latte




#endif