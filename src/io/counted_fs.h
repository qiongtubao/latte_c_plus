

#include "file_system_wrapper.h"

namespace latte
{
    struct FileOpCounters {
        static const char* kName() { return "FileOpCounters"; }
        
    };

    class CountedFileSystem : public FileSystemWrapper {
        public:
        private:
            FileOpCounters counters_;
    };
} // namespace latte
