



#ifndef __LATTE_C_PLUS_IO_FS_READONLY_H
#define __LATTE_C_PLUS_IO_FS_READONLY_H

#include "file_system_wrapper.h"
#include "io_status.h"
#include <cassert>

namespace latte
{
    class ReadOnlyFileSystem: public FileSystemWrapper {
        static inline IOStatus FailReadOnly() {
            IOStatus s = IOStatus::IOError("Attempted write to ReadOnlyFileSystem");
            assert(s.GetRetryable() == false);
            return s;
        }
        public:
            explicit ReadOnlyFileSystem(const std::shared_ptr<FileSystem>& base)
                : FileSystemWrapper(base) { }
            static const char* kClassName() { return "ReadOnlyFileSystem"; }
            const char* Name() const override { return kClassName(); }
    };
} // namespace latte


#endif