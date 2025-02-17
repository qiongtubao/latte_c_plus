
#include "file_system_wrapper.h"

namespace latte
{
    class TimedFileSystem : public FileSystemWrapper {
        public:
            explicit TimedFileSystem(const std::shared_ptr<FileSystem>& base);

            static const char* kClassName() { return "TimedFS"; }

            const char* Name() const override { return kClassName(); }

    };
} // namespace latte
