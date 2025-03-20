#pragma once
#include <string>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include "slice/slice.h"


namespace latte
{
    // Standard Put... routines append to a string
    void PutFixed32(std::string* dst, uint32_t value);
    void PutFixed64(std::string* dst, uint64_t value);
    void PutVarint32(std::string* dst, uint32_t value);
    void PutVarint64(std::string* dst, uint64_t value);
    void PutLengthPrefixedSlice(std::string* dst, const Slice& value);

    // Standard Get... routines parse a value from the beginning of a Slice
    // and advance the slice past the parsed value.
    bool GetVarint32(Slice* input, uint32_t* value);
    bool GetVarint64(Slice* input, uint64_t* value);
    bool GetLengthPrefixedSlice(Slice* input, Slice* result);

    // Pointer-based variants of GetVarint...  These either store a value
    // in *v and return a pointer just past the parsed value, or return
    // nullptr on error.  These routines only look at bytes in the range
    // [p..limit-1]
    const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* v);
    const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* v);

    // Returns the length of the varint32 or varint64 encoding of "v"
    int VarintLength(uint64_t v);

    // Lower-level versions of Put... that write directly into a character buffer
    // and return a pointer just past the last byte written.
    // REQUIRES: dst has enough space for the value being written
    char* EncodeVarint32(char* dst, uint32_t value);
    char* EncodeVarint64(char* dst, uint64_t value);
    
    // 32位定长数据编码
    inline void EncodeFixed32(char* dst, uint32_t value);

    //64位定长数据编码
    inline void EncodeFixed64(char* dst, uint64_t value);

    //32位定长解码
    inline uint32_t DecodeFixed32(const char* ptr);

    //定长64位解码
    inline uint64_t DecodeFixed64(const char* ptr) ;

    // Internal routine for use by fallback path of GetVarint32Ptr
    const char* GetVarint32PtrFallback(const char* p, const char* limit,
                                    uint32_t* value);
    inline const char* GetVarint32Ptr(const char* p, const char* limit,
                                    uint32_t* value) ;

} // namespace latte
