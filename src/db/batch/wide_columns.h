




#ifndef __LATTE_C_PLUS_WIDE_COLUMNS_H
#define __LATTE_C_PLUS_WIDE_COLUMNS_H

#include <vector>
#include "slice/slice.h"

namespace latte
{
    namespace leveldb
    {
        
    } // namespace leveldb

    namespace rocksdb
    {
        class WideColumn {
            public:
                WideColumn() = default;
                // 通过将名称和值参数转发给相应的成员 Slices 来初始化 WideColumn 对象。这样就可以使用 const char*、const
                // std::string&、const Slice& 等的组合构造 WideColumn，例如：
                //
                // constexpr char foo[] = "foo";
                // const std::string bar("bar");
                // WideColumn column(foo, bar);
                template <typename N, typename V>
                WideColumn(N&& name, V&& value)
                    : name_(std::forward<N>(name)), value_(std::forward<V>(value)) {}

                // 通过将 name_tuple 和 value_tuple 的元素转发到相应成员 Slices 的构造函数来初始化 WideColumn 对象。这样就可以使用接受多个参数的 Slice 构造函数来初始化 Slices，例如：
                //
                // constexpr char foo_name[] = "foo_name";
                // constexpr char bar_value[] = "bar_value";
                // WideColumn column(std::piecewise_construct,
                // std::forward_as_tuple(foo_name, 3),
                // std::forward_as_tuple(bar_value, 3));
                template <typename NTuple, typename VTuple>
                WideColumn(std::piecewise_construct_t, NTuple&& name_tuple,
                            VTuple&& value_tuple)
                    : name_(std::make_from_tuple<Slice>(std::forward<NTuple>(name_tuple))),
                        value_(std::make_from_tuple<Slice>(std::forward<VTuple>(value_tuple))) {
                }

                const Slice& name() const { return name_; }
                const Slice& value() const { return value_; }

                Slice& name() { return name_; }
                Slice& value() { return value_; }

            private:
                Slice name_;
                Slice value_;
        };
        // A collection of wide columns.
        using WideColumns = std::vector<WideColumn>;
    } // namespace rocksdb
    
} // namespace latte

#endif