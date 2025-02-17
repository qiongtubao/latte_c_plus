


#ifndef __LATTE_C_PLUS_ITER_KEY_H
#define __LATTE_C_PLUS_ITER_KEY_H

#include <cstdint>
#include <string>
#include "slice/slice.h"
#include <cassert>
#include "../types.h"
#include "./config.h"
#include "./parsed_internal_key.h"
#include "./format.h"

namespace latte
{

    namespace rocksdb {
        // The class to store keys in an efficient way. It allows:
        // 1. Users can either copy the key into it, or have it point to an unowned
        //    address.
        // 2. For copied key, a short inline buffer is kept to reduce memory
        //    allocation for smaller keys.
        // 3. It tracks user key or internal key, and allow conversion between them.
        class IterKey {
            static constexpr size_t kInlineBufferSize = 39;
            // This is only used by user-defined timestamps in MemTable only feature,
            // which only supports uint64_t timestamps.
            static constexpr char kTsMin[] = "\x00\x00\x00\x00\x00\x00\x00\x00";
            public:
                IterKey()
                    : buf_(space_),
                        key_(buf_),
                        key_size_(0),
                        buf_size_(kInlineBufferSize),
                        is_user_key_(true),
                        secondary_buf_(space_for_secondary_buf_),
                        secondary_buf_size_(kInlineBufferSize) {}
                // No copying allowed
                IterKey(const IterKey&) = delete;
                void operator=(const IterKey&) = delete;

                ~IterKey() {
                    ResetBuffer();
                    ResetSecondaryBuffer();
                }

                // The bool will be picked up by the next calls to SetKey
                void SetIsUserKey(bool is_user_key) { is_user_key_ = is_user_key; }

                // Returns the key in whichever format that was provided to KeyIter
                // If user-defined timestamp is enabled, then timestamp is included in the
                // return result.
                Slice GetKey() const { return Slice(key_, key_size_); }

                Slice GetInternalKey() const {
                    assert(!IsUserKey());
                    return Slice(key_, key_size_);
                }

                // If user-defined timestamp is enabled, then timestamp is included in the
                // return result of GetUserKey();
                Slice GetUserKey() const {
                    if (IsUserKey()) {
                    return Slice(key_, key_size_);
                    } else {
                    assert(key_size_ >= kNumInternalBytes);
                    return Slice(key_, key_size_ - kNumInternalBytes);
                    }
                }

                size_t Size() const { return key_size_; }

                void Clear() { key_size_ = 0; }

                // Append "non_shared_data" to its back, from "shared_len"
                // This function is used in Block::Iter::ParseNextKey
                // shared_len: bytes in [0, shard_len-1] would be remained
                // non_shared_data: data to be append, its length must be >= non_shared_len
                void TrimAppend(const size_t shared_len, const char* non_shared_data,
                                const size_t non_shared_len) {
                    assert(shared_len <= key_size_);
                    size_t total_size = shared_len + non_shared_len;

                    if (IsKeyPinned() /* key is not in buf_ */) {
                        // Copy the key from external memory to buf_ (copy shared_len bytes)
                        EnlargeBufferIfNeeded(total_size);
                        memcpy(buf_, key_, shared_len);
                    } else if (total_size > buf_size_) {
                        // Need to allocate space, delete previous space
                        char* p = new char[total_size];
                        memcpy(p, key_, shared_len);

                        if (buf_ != space_) {
                            delete[] buf_;
                        }

                        buf_ = p;
                        buf_size_ = total_size;
                    }

                    memcpy(buf_ + shared_len, non_shared_data, non_shared_len);
                    key_ = buf_;
                    key_size_ = total_size;
                }

                // A version of `TrimAppend` assuming the last bytes of length `ts_sz` in the
                // user key part of `key_` is not counted towards shared bytes. And the
                // decoded key needed a min timestamp of length `ts_sz` pad to the user key.
                void TrimAppendWithTimestamp(const size_t shared_len,
                                            const char* non_shared_data,
                                            const size_t non_shared_len,
                                            const size_t ts_sz) {
                    // This function is only used by the UDT in memtable feature, which only
                    // support built in comparators with uint64 timestamps.
                    assert(ts_sz == sizeof(uint64_t));
                    size_t next_key_slice_index = 0;
                    if (IsUserKey()) {
                        key_slices_[next_key_slice_index++] = Slice(key_, shared_len);
                        key_slices_[next_key_slice_index++] =
                            Slice(non_shared_data, non_shared_len);
                        key_slices_[next_key_slice_index++] = Slice(kTsMin, ts_sz);
                    } else {
                        assert(shared_len + non_shared_len >= kNumInternalBytes);
                        // Invaraint: shared_user_key_len + shared_internal_bytes_len = shared_len
                        // In naming below `*_len` variables, keyword `user_key` refers to the
                        // user key part of the existing key in `key_` as apposed to the new key.
                        // Similary, `internal_bytes` refers to the footer part of the existing
                        // key. These bytes potentially will move between user key part and the
                        // footer part in the new key.
                        const size_t user_key_len = key_size_ - kNumInternalBytes;
                        const size_t sharable_user_key_len = user_key_len - ts_sz;
                        const size_t shared_user_key_len =
                            std::min(shared_len, sharable_user_key_len);
                        const size_t shared_internal_bytes_len = shared_len - shared_user_key_len;

                        // One Slice among the three Slices will get split into two Slices, plus
                        // a timestamp slice.
                        bool ts_added = false;
                        // Add slice parts and find the right location to add the min timestamp.
                        MaybeAddKeyPartsWithTimestamp(
                            key_, shared_user_key_len,
                            shared_internal_bytes_len + non_shared_len < kNumInternalBytes,
                            shared_len + non_shared_len - kNumInternalBytes, ts_sz,
                            &next_key_slice_index, &ts_added);
                        MaybeAddKeyPartsWithTimestamp(
                            key_ + user_key_len, shared_internal_bytes_len,
                            non_shared_len < kNumInternalBytes,
                            shared_internal_bytes_len + non_shared_len - kNumInternalBytes, ts_sz,
                            &next_key_slice_index, &ts_added);
                        MaybeAddKeyPartsWithTimestamp(non_shared_data, non_shared_len,
                                                        non_shared_len >= kNumInternalBytes,
                                                        non_shared_len - kNumInternalBytes, ts_sz,
                                                        &next_key_slice_index, &ts_added);
                        assert(ts_added);
                    }
                    SetKeyImpl(next_key_slice_index,
                            /* total_bytes= */ shared_len + non_shared_len + ts_sz);
                }

                Slice SetKeyWithPaddedMinTimestamp(const Slice& key, size_t ts_sz) {
                    // This function is only used by the UDT in memtable feature, which only
                    // support built in comparators with uint64 timestamps.
                    assert(ts_sz == sizeof(uint64_t));
                    size_t num_key_slices = 0;
                    if (is_user_key_) {
                        key_slices_[0] = key;
                        key_slices_[1] = Slice(kTsMin, ts_sz);
                        num_key_slices = 2;
                    } else {
                        assert(key.size() >= kNumInternalBytes);
                        size_t user_key_size = key.size() - kNumInternalBytes;
                        key_slices_[0] = Slice(key.data(), user_key_size);
                        key_slices_[1] = Slice(kTsMin, ts_sz);
                        key_slices_[2] = Slice(key.data() + user_key_size, kNumInternalBytes);
                        num_key_slices = 3;
                    }
                    return SetKeyImpl(num_key_slices, key.size() + ts_sz);
                }

                Slice SetKey(const Slice& key, bool copy = true) {
                    // is_user_key_ expected to be set already via SetIsUserKey
                    return SetKeyImpl(key, copy);
                }

                // If user-defined timestamp is enabled, then `key` includes timestamp.
                // TODO(yanqin) this is also used to set prefix, which do not include
                // timestamp. Should be handled.
                Slice SetUserKey(const Slice& key, bool copy = true) {
                    is_user_key_ = true;
                    return SetKeyImpl(key, copy);
                }

                Slice SetInternalKey(const Slice& key, bool copy = true) {
                    is_user_key_ = false;
                    return SetKeyImpl(key, copy);
                }

                // Copies the content of key, updates the reference to the user key in ikey
                // and returns a Slice referencing the new copy.
                Slice SetInternalKey(const Slice& key, ParsedInternalKey* ikey) {
                    size_t key_n = key.size();
                    assert(key_n >= kNumInternalBytes);
                    SetInternalKey(key);
                    ikey->user_key = Slice(key_, key_n - kNumInternalBytes);
                    return Slice(key_, key_n);
                }

                // Update the sequence number in the internal key.  Guarantees not to
                // invalidate slices to the key (and the user key).
                void UpdateInternalKey(uint64_t seq, ValueType t, const Slice* ts = nullptr) {
                    assert(!IsKeyPinned());
                    assert(key_size_ >= kNumInternalBytes);
                    if (ts) {
                        assert(key_size_ >= kNumInternalBytes + ts->size());
                        memcpy(&buf_[key_size_ - kNumInternalBytes - ts->size()], ts->data(),
                                ts->size());
                    }
                    uint64_t newval = (seq << 8) | t;
                    if (key_ == buf_) {
                        EncodeFixed64(&buf_[key_size_ - kNumInternalBytes], newval);
                    } else {
                    assert(key_ == secondary_buf_);
                        EncodeFixed64(&secondary_buf_[key_size_ - kNumInternalBytes], newval);
                    }
                }

                bool IsKeyPinned() const { return key_ != buf_ && key_ != secondary_buf_; }

                // If `ts` is provided, user_key should not contain timestamp,
                // and `ts` is appended after user_key.
                // TODO: more efficient storage for timestamp.
                void SetInternalKey(const Slice& key_prefix, const Slice& user_key,
                                    SequenceNumber s,
                                    ValueType value_type = kValueTypeForSeek,
                                    const Slice* ts = nullptr) {
                    size_t psize = key_prefix.size();
                    size_t usize = user_key.size();
                    size_t ts_sz = (ts != nullptr ? ts->size() : 0);
                    EnlargeBufferIfNeeded(psize + usize + sizeof(uint64_t) + ts_sz);
                    if (psize > 0) {
                        memcpy(buf_, key_prefix.data(), psize);
                    }
                        memcpy(buf_ + psize, user_key.data(), usize);
                    if (ts) {
                        memcpy(buf_ + psize + usize, ts->data(), ts_sz);
                    }
                    EncodeFixed64(buf_ + usize + psize + ts_sz,
                                PackSequenceAndType(s, value_type));

                    key_ = buf_;
                    key_size_ = psize + usize + sizeof(uint64_t) + ts_sz;
                    is_user_key_ = false;
                }

                void SetInternalKey(const Slice& user_key, SequenceNumber s,
                                    ValueType value_type = kValueTypeForSeek,
                                    const Slice* ts = nullptr) {
                    SetInternalKey(Slice(), user_key, s, value_type, ts);
                }

                void Reserve(size_t size) {
                    EnlargeBufferIfNeeded(size);
                    key_size_ = size;
                }

                void SetInternalKey(const ParsedInternalKey& parsed_key) {
                    SetInternalKey(Slice(), parsed_key);
                }

                void SetInternalKey(const Slice& key_prefix,
                                    const ParsedInternalKey& parsed_key_suffix) {
                    SetInternalKey(key_prefix, parsed_key_suffix.user_key,
                                parsed_key_suffix.sequence, parsed_key_suffix.type);
                }

                void EncodeLengthPrefixedKey(const Slice& key) {
                    auto size = key.size();
                    EnlargeBufferIfNeeded(size + static_cast<size_t>(VarintLength(size)));
                    char* ptr = EncodeVarint32(buf_, static_cast<uint32_t>(size));
                    memcpy(ptr, key.data(), size);
                    key_ = buf_;
                    is_user_key_ = true;
                }

                bool IsUserKey() const { return is_user_key_; }

                private:
                char* buf_;
                const char* key_;
                size_t key_size_;
                size_t buf_size_;
                char space_[kInlineBufferSize];  // Avoid allocation for short keys
                bool is_user_key_;
                // Below variables are only used by user-defined timestamps in MemTable only
                // feature for iterating keys in an index block or a data block.
                //
                // We will alternate between buf_ and secondary_buf_ to hold the key. key_
                // will be modified in accordance to point to the right one. This is to avoid
                // an extra copy when we need to copy some shared bytes from previous key
                // (delta encoding), and we need to pad a min timestamp at the right location.
                char space_for_secondary_buf_[kInlineBufferSize];  // Avoid allocation for
                                                                    // short keys
                char* secondary_buf_;
                size_t secondary_buf_size_;
                // Use to track the pieces that together make the whole key. We then copy
                // these pieces in order either into buf_ or secondary_buf_ depending on where
                // the previous key is held.
                std::array<Slice, 5> key_slices_;
                // End of variables used by user-defined timestamps in MemTable only feature.

                Slice SetKeyImpl(const Slice& key, bool copy) {
                    size_t size = key.size();
                    if (copy) {
                    // Copy key to buf_
                    EnlargeBufferIfNeeded(size);
                    memcpy(buf_, key.data(), size);
                    key_ = buf_;
                    } else {
                    // Update key_ to point to external memory
                    key_ = key.data();
                    }
                    key_size_ = size;
                    return Slice(key_, key_size_);
                }

                Slice SetKeyImpl(size_t num_key_slices, size_t total_bytes) {
                    assert(num_key_slices <= 5);
                    char* buf_start = nullptr;
                    if (key_ == buf_) {
                    // If the previous key is in buf_, we copy key_slices_ in order into
                    // secondary_buf_.
                    EnlargeSecondaryBufferIfNeeded(total_bytes);
                    buf_start = secondary_buf_;
                    key_ = secondary_buf_;
                    } else {
                    // Copy key_slices_ in order into buf_.
                    EnlargeBufferIfNeeded(total_bytes);
                    buf_start = buf_;
                    key_ = buf_;
                    }
                #ifndef NDEBUG
                    size_t actual_total_bytes = 0;
                #endif  // NDEBUG
                    for (size_t i = 0; i < num_key_slices; i++) {
                    size_t key_slice_size = key_slices_[i].size();
                    memcpy(buf_start, key_slices_[i].data(), key_slice_size);
                    buf_start += key_slice_size;
                #ifndef NDEBUG
                    actual_total_bytes += key_slice_size;
                #endif  // NDEBUG
                    }
                #ifndef NDEBUG
                    assert(actual_total_bytes == total_bytes);
                #endif  // NDEBUG
                    key_size_ = total_bytes;
                    return Slice(key_, key_size_);
                }

                void ResetBuffer() {
                    if (key_ == buf_) {
                    key_size_ = 0;
                    }
                    if (buf_ != space_) {
                    delete[] buf_;
                    buf_ = space_;
                    }
                    buf_size_ = kInlineBufferSize;
                }

                void ResetSecondaryBuffer() {
                    if (key_ == secondary_buf_) {
                    key_size_ = 0;
                    }
                    if (secondary_buf_ != space_for_secondary_buf_) {
                    delete[] secondary_buf_;
                    secondary_buf_ = space_for_secondary_buf_;
                    }
                    secondary_buf_size_ = kInlineBufferSize;
                }

                // Enlarge the buffer size if needed based on key_size.
                // By default, inline buffer is used. Once there is a key
                // larger than the inline buffer, another buffer is dynamically
                // allocated, until a larger key buffer is requested. In that case, we
                // reallocate buffer and delete the old one.
                void EnlargeBufferIfNeeded(size_t key_size) {
                    // If size is smaller than buffer size, continue using current buffer,
                    // or the static allocated one, as default
                    if (key_size > buf_size_) {
                    EnlargeBuffer(key_size);
                    }
                }

                void EnlargeSecondaryBufferIfNeeded(size_t key_size);

                void EnlargeBuffer(size_t key_size);

                void MaybeAddKeyPartsWithTimestamp(const char* slice_data,
                                                    const size_t slice_sz, bool add_timestamp,
                                                    const size_t left_sz, const size_t ts_sz,
                                                    size_t* next_key_slice_idx,
                                                    bool* ts_added) {
                    assert(next_key_slice_idx);
                    if (add_timestamp && !*ts_added) {
                    assert(slice_sz >= left_sz);
                    key_slices_[(*next_key_slice_idx)++] = Slice(slice_data, left_sz);
                    key_slices_[(*next_key_slice_idx)++] = Slice(kTsMin, ts_sz);
                    key_slices_[(*next_key_slice_idx)++] =
                        Slice(slice_data + left_sz, slice_sz - left_sz);
                    *ts_added = true;
                    } else {
                    key_slices_[(*next_key_slice_idx)++] = Slice(slice_data, slice_sz);
                    }
                    assert(*next_key_slice_idx <= 5);
                }
        };
    };
    
} // namespace latte




#endif