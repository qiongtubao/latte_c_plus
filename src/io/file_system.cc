

#include "file_system.h"
#include "file_system_wrapper.h"
#include "status/status.h"
#include "posix_file_system.h"
#include "object_library.h"
#include "timed.h"
#include "fs_readonly.h"
#include "cryption_ctr.h"


extern "C" bool RocksDbIOUringEnable() __attribute__((__weak__));

namespace latte
{

    static int RegisterBuiltinFileSystems(ObjectLibrary& library,
                                    const std::string& /*arg*/) {
        library.AddFactory<FileSystem>(
            TimeFileSystem::kClassName(),
            [](const std::string& /*uri*/, std::unique_ptr<FileSystem>* guard,
                std::string* /*errmsg*/) {
                guard->reset(new TimedFileSystem(nullptr));
                return guard->get();
            });
        library.AddFactory<FileSystem>(
            ReadOnlyFileSystem::kClassName(),
            [](const std::string& /*uri*/, std::unique_ptr<FileSystem>* guard,
                std::string* /*errmsg*/ ) {
                    guard->reset(new ReadOnlyFileSystem(nullptr));
                    return guard->get();
            });
        library.AddFactory<FileSystem>(
            EncryptedFileSystem::kClassName(),
            [](const std::string& /*uri*/, std::unique_ptr<FileSystem>* guard,
                std::string* errmsg) {
                Status s = NewEncryptedFileSystemImpl(nullptr, nullptr, guard);
                if (!s.ok()) {
                    *errmsg = s.ToString();
                }
                return guard->get();
            });
        library.AddFactory<FileSystem>(
            CountedFileSystem::kClassName(),
            [](const std::string& /*uri*/, std::unique_ptr<FileSystem>* guard,
                std::string* /*errmsg*/) {
                guard->reset(new CountedFileSystem(FileSystem::Default()));
                return guard->get();    
            });
        #ifndef OS_WIN
            
        #endif // OS_WIN
        size_t num_types;
        return static_cast<int>(library.GetFactoryCount(&num_types));
    };

    static std::shared_ptr<FileSystem> Default() {
        STATIC_AVOID_DESTRUCTION(std::shared_ptr<FileSystem>, instance)
        (std::make_shared<PosixFileSystem>());
        return instance;
    }

    Status FileSystem::CreateFromString(const ConfigOptions& config_options,
                                    const std::string& value,
                                    std::shared_ptr<FileSystem>* result) {
        auto default_fs = FileSystem::Default();
        if (default_fs->IsInstanceOf(value)) {
            *result = default_fs;
            return Status::OK();
        } else {
            static std::once_flag once;
            std::call_once(once, [&]() {
                RegisterBuiltinFileSystems(*(ObjectLibrary::Default().get()), "");
            });
            return LoadSharedObject<FileSystem>(config_options, value, result);
        }
    };
} // namespace latte
