

#include <vector>
#include "slice.h"

namespace latte
{
    class Slice;

    std::vector<std::string> StringSplit(const std::string& arg, char delim);

    // 将“num”的可读打印输出附加到 *str
    void AppendNumberTo(std::string* str, uint64_t num);
    // 将“value”的可读打印输出附加到 *str。
    // 转义“value”中发现的任何不可打印字符。
    void AppendEscapedStringTo(std::string* str, const Slice& value);

    bool ConsumeDecimalNumber(Slice* in, uint64_t* val);

} // namespace latte
