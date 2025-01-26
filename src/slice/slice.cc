


#include "slice.h"

namespace latte
{
    // class FixedPrefixTransform : public SliceTransform {
    //     private:
    //         size_t prefix_len_;
    //         std::string id_;

    // };   
    char toHex(unsigned char v) {
        if (v <= 9) {
            return '0' + v;
        }
        return 'A' + v - 10;
    }

    int fromHex(char c) {
        if (c >= 'a' && c <= 'f') {
            c -= ('a' - 'A');
        }

        if (c < '0' || (c > '9' && (c < 'A' || c > 'F'))) {
            return -1;
        }
        
        if (c <= '9') {
            return c - '0';
        }
        return c - 'A' + 10;
    }

    std::string Slice::ToString(bool hex) const {
        std::string result;  // RVO/NRVO/move
        if (hex) {
            result.reserve(2 * size_);
            for (size_t i = 0; i < size_; ++i) {
                unsigned char c = data_[i];
                result.push_back(toHex(c >> 4));
                result.push_back(toHex(c & 0xf));
            }
            return result;
        } else {
            result.assign(data_, size_);
            return result;
        }
    }
} // namespace latte
