


#include "status/status.h"
#include "file_system.h"
#include "encryption.h"

namespace latte
{

   

    Status NewEncryptedFileSystemImpl(
        const std::shared_ptr<FileSystem>& base_fs,
        const std::shared_ptr<EncryptionProvider>& provider,
        std::unique_ptr<FileSystem>* fs);
} // namespace latte
