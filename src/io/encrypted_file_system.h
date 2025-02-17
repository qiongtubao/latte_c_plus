


#include "file_system_wrapper.h"


namespace latte
{
    

    class EncryptedFileSystem: public FileSystemWrapper {
        public:
            explicit EncryptedFileSystem(const std::shared_ptr<FileSystem>& base)
                : FileSystemWrapper(base) {}
    };
} // namespace latte
