
#ifndef __LATTE_C_PLUS_COMPARATROR_H
#define __LATTE_C_PLUS_COMPARATROR_H

#include "slice/slice.h"
#include "customizable/customizable.h"
namespace latte
{
    class CompareInterface {
        public:
            virtual ~CompareInterface() {}
            // Three-way comparison.  Returns value:
            //   < 0 iff "a" < "b",
            //   == 0 iff "a" == "b",
            //   > 0 iff "a" > "b"
            // Note that Compare(a, b) also compares timestamp if timestamp size is
            // non-zero. For the same user key with different timestamps, larger (newer)
            // timestamp comes first.
            virtual int Compare(const Slice& a, const Slice& b) const = 0;
    };


    class Comparator: public Customizable,  public CompareInterface {
        public: 
            static Status CreateFromString(const ConfigOptions& opts,
                                 const std::string& id,
                                 const Comparator** comp);
            static const char* Type() { return "Comparator"; }

        public:
            Comparator() : timestamp_size_(0) {}

            Comparator(size_t ts_sz) : timestamp_size_(ts_sz) {}

            Comparator(const Comparator& orig) : timestamp_size_(orig.timestamp_size_) {}

            Comparator& operator=(const Comparator& rhs) {
                if (this != &rhs) {
                timestamp_size_ = rhs.timestamp_size_;
                }
                return *this;
            }

            ~Comparator() override {}

            // 比较器的名称。用于检查比较器
            // 是否不匹配（即，使用一个比较器创建的数据库
            // 使用不同的比较器进行访问。
            //
            // 此包的客户端应在比较器实现发生更改时切换到新名称，从而导致
            // 任何两个键的相对顺序发生更改。
            //
            // 以“rocksdb.”开头的名称是保留的，不应由此包的任何客户端使用。
            const char* Name() const override = 0;

            // 比较两个切片是否相等。以下不变量应始终
            // 保持（并且是默认实现）：
            // Equal(a, b) 当且仅当 Compare(a, b) == 0
            // 仅当相等比较比
            // 三向比较更有效时才覆盖。
            virtual bool Equal(const Slice& a, const Slice& b) const {
                return Compare(a, b) == 0;
            }
        


        private:
            size_t timestamp_size_;

    };

} // namespace latte

#endif