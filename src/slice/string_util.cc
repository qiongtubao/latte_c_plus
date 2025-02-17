


#include <sstream>
#include "string_util.h"
#include "inttypes.h"

namespace latte
{
    const std::string kNullptrString = "nullptr";

    std::vector<std::string> StringSplit(const std::string& arg, char delim) {
        std::vector<std::string> splits;
        std::stringstream ss(arg);
        std::string item;
        while (std::getline(ss, item, delim)) {
            splits.push_back(item);
        }
        return splits;
    }

    void AppendNumberTo(std::string* str, uint64_t num) {
        char buf[30];
        snprintf(buf, sizeof(buf), "%" PRIu64, num);
        str->append(buf);
    }

    void AppendEscapedStringTo(std::string* str, const Slice& value) {
        for (size_t i = 0; i < value.size(); i++) {
            char c = value[i];
            if (c >= ' ' && c <= '~') {
                str->push_back(c);
            } else {
                char buf[10];
                snprintf(buf,  sizeof(buf), "\\x%02x",
                    static_cast<unsigned int>(c) & 0xff);
                str->append(buf);
            }
        }
    }

    
} // namespace latte
