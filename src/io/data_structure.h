

#ifndef __LATTE_C_PLUS_DATA_STRUCTURE_H
#define __LATTE_C_PLUS_DATA_STRUCTURE_H

#include "types.h"
namespace latte
{
    template <typename ENUM_TYPE, ENUM_TYPE MAX_ENUMERATOR>
    class SmallEnumSet {
        private:
            using StateT = uint64_t;
            static constexpr int kStateBits = sizeof(StateT) * 8;
            static constexpr int kMaxMax = kStateBits - 1;
            static constexpr int kMaxValue = static_cast<int>(MAX_ENUMERATOR);
            static_assert(kMaxValue >= 0);
            static_assert(kMaxValue <= kMaxMax);

        public:
            // construct / create
            SmallEnumSet() : state_(0) {}

            template <class... TRest>
            /*implicit*/ constexpr SmallEnumSet(const ENUM_TYPE e, TRest... rest) {
                *this = SmallEnumSet(rest...).With(e);
            }

            // Return the set that includes all valid values, assuming the enum
            // is "dense" (includes all values converting to 0 through kMaxValue)
            static constexpr SmallEnumSet All() {
                StateT tmp = StateT{1} << kMaxValue;
                return SmallEnumSet(RawStateMarker(), tmp | (tmp - 1));
            }

            // equality
            bool operator==(const SmallEnumSet& that) const {
                return this->state_ == that.state_;
            }
            bool operator!=(const SmallEnumSet& that) const { return !(*this == that); }

            // query

            // Return true if the input enum is contained in the "Set".
            bool Contains(const ENUM_TYPE e) const {
                int value = static_cast<int>(e);
                assert(value >= 0 && value <= kMaxValue);
                StateT tmp = 1;
                return state_ & (tmp << value);
            }

            bool empty() const { return state_ == 0; }

        // iterator
        class const_iterator {
            public:
                // copy
                const_iterator(const const_iterator& that) = default;
                const_iterator& operator=(const const_iterator& that) = default;

                // move
                const_iterator(const_iterator&& that) noexcept = default;
                const_iterator& operator=(const_iterator&& that) noexcept = default;

                // equality
                bool operator==(const const_iterator& that) const {
                    assert(set_ == that.set_);
                    return this->pos_ == that.pos_;
                }

                bool operator!=(const const_iterator& that) const {
                    return !(*this == that);
                }

                // ++iterator
                const_iterator& operator++() {
                    if (pos_ < kMaxValue) {
                        pos_ = set_->SkipUnset(pos_ + 1);
                    } else {
                        pos_ = kStateBits;
                    }
                    return *this;
                }

                // iterator++
                const_iterator operator++(int) {
                    auto old = *this;
                    ++*this;
                    return old;
                }

                ENUM_TYPE operator*() const {
                    assert(pos_ <= kMaxValue);
                    return static_cast<ENUM_TYPE>(pos_);
                }

            private:
                friend class SmallEnumSet;
                const_iterator(const SmallEnumSet* set, int pos) : set_(set), pos_(pos) {}
                const SmallEnumSet* set_;
                int pos_;
        };
    };
    using FileTypeSet = SmallEnumSet<FileType, FileType::kBlobFile>;
} // namespace latte

#endif