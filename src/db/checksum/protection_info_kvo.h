


#ifndef __LATTE_C_PLUS_PROTECTION_INFO_KVO_H
#define __LATTE_C_PLUS_PROTECTION_INFO_KVO_H


namespace latte
{
    namespace rocksdb
    {
        
        template <typename T>
            class ProtectionInfoKVO {
                public:
                    ProtectionInfoKVO() = default;

                    ProtectionInfo<T> StripKVO(const Slice& key, const Slice& value,
                                                ValueType op_type) const;
                    ProtectionInfo<T> StripKVO(const SliceParts& key, const SliceParts& value,
                                                ValueType op_type) const;

                    ProtectionInfoKVOC<T> ProtectC(ColumnFamilyId column_family_id) const;
                    ProtectionInfoKVOS<T> ProtectS(SequenceNumber sequence_number) const;

                    void UpdateK(const Slice& old_key, const Slice& new_key);
                    void UpdateK(const SliceParts& old_key, const SliceParts& new_key);
                    void UpdateV(const Slice& old_value, const Slice& new_value);
                    void UpdateV(const SliceParts& old_value, const SliceParts& new_value);
                    void UpdateO(ValueType old_op_type, ValueType new_op_type);

                    // Encode this protection info into `len` bytes and stores them in `dst`.
                    void Encode(uint8_t len, char* dst) const { info_.Encode(len, dst); }
                    // Verify this protection info against the protection info encoded by Encode()
                    // at the first `len` bytes of `checksum_ptr`.
                    // Returns true iff the verification is successful.
                    bool Verify(uint8_t len, const char* checksum_ptr) const {
                        return info_.Verify(len, checksum_ptr);
                    }

                private:
                    friend class ProtectionInfo<T>;
                    friend class ProtectionInfoKVOS<T>;
                    friend class ProtectionInfoKVOC<T>;

                    explicit ProtectionInfoKVO(T val) : info_(val) {
                        static_assert(sizeof(ProtectionInfoKVO<T>) == sizeof(T), "");
                    }

                    T GetVal() const { return info_.GetVal(); }
                    void SetVal(T val) { info_.SetVal(val); }

                    ProtectionInfo<T> info_;
            };

    } // namespace rocksdb
    
} // namespace latte


#endif