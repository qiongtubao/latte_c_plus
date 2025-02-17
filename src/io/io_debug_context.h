

#include <string>
#include <map>

namespace latte
{
    struct IODebugContext {
        std::string file_path;

        std::map<std::string, uint64_t> counters;

        std::string msg;

        std::string request_id;

        enum TraceData : char {
            kRequestID = 0,
        };

        uint64_t trace_data = 0;

        IODebugContext() {}

        

    };   
} // namespace latte
