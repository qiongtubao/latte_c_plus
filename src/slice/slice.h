
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>



namespace latte
{
    class Slice {
        public:
            Slice() : data_(""), size_(0) {}

            Slice(const char* d, size_t n) : data_(d), size_(n) {}

            Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}

            Slice(const std::string_view& sv) : data_(sv.data()), size_(sv.size()) {}

            Slice(const char* s) : data_(s) { size_ = (s == nullptr) ? 0 : strlen(s); }

            Slice(const struct SliceParts& parts, std::string* buf);

            size_t size() const { return size_; }

            const char* data() const { return data_; }

            bool empty() const { return size_ == 0; }

            char operator[](size_t n) const {
                assert(n < size());
                return data_[n];
            }

            void clear() {
                data_ = "";
                size_ = 0;
            }

            void remove_prefix(size_t n) {
                assert(n <= size());
                data_ += n;
                size_ -= n;
            }

            void remove_suffix(size_t n) {
                assert(n <= size());
                size_ -= n;
            }

        // 返回包含引用数据副本的字符串。
        // 当 hex 为真时，返回两倍长度的十六进制编码的字符串（0-9A-F）
        std::string ToString(bool hex) const;

        // 返回引用与此切片相同数据的 string_view。
        std::string_view ToStringView() const {
            return std::string_view(data_, size_);
        }

        // 将当前切片解码为十六进制字符串，结果为
        // 如果成功则返回 true，如果这不是有效的十六进制字符串
        // （例如不是来自 Slice::ToString(true)）DecodeHex 返回 false。
        // 此切片应具有偶数个 0-9A-F 字符
        // 也接受小写字母（a-f）
        bool DecodeHex(std::string* result) const;

        // 三向比较。返回值：
        // < 0 当且仅当“*this”<“b”，
        // == 0 当且仅当“*this”==“b”，
        // > 0 当且仅当“*this”>“b”
        int compare(const Slice& b) const;

        // Return true iff "x" is a prefix of "*this"
        bool starts_with(const Slice& x) const {
            return ((size_ >= x.size_) && (memcmp(data_, x.data_, x.size_) == 0));
        }

        bool ends_with(const Slice& x) const {
            return ((size_ >= x.size_) &&
                    (memcmp(data_ + size_ - x.size_, x.data_, x.size_) == 0));
        }

        // Compare two slices and returns the first byte where they differ
        size_t difference_offset(const Slice& b) const;

        // private:
            const char* data_;
            size_t size_;
    };
} // namespace latte
