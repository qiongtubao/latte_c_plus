

#include "file_system.h"
#include "env_encryption_ctr.h"

namespace latte
{

    class EncryptedFileSystemImpl: public EncryptedFileSystem {
        public:
            const char* Name() const override {
                return EncryptedFileSystem::kClassName();
            }
            
    };

    Status NewEncryptedFileSystemImpl(
        const std::shared_ptr<FileSystem>& base,
        const std::shared_ptr<EncryptionProvider>& provider,
        std::unique_ptr<FileSystem>* result) {
        result->reset(new EncryptedFileSystemImpl(base, provider));
        return Status::OK();
    }

} // namespace latte
