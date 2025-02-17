

#include "file_system.h"

namespace latte
{
    class PosixFileSystem : public FileSystem {
        public:
            PosixFileSystem();
            static const char* kClassName() { return "PosixFileSystem"; }
            const char* Name() const override { return kClassName(); }
            const char* NickName() const override { return kDefaultName(); }

            ~PosixFileSystem() override = default;
            bool IsInstanceOf(const std::string& name) const override {
                if ( name == "posix") {
                    return true;
                } else {
                    return FileSystem::IsInstanceOf(name);
                }
            }
    };
} // namespace latte

