

#ifndef __LATTE_C_PLUS_ENV_FILE_SYSTEM_WRAPPER_H
#define __LATTE_C_PLUS_ENV_FILE_SYSTEM_WRAPPER_H

#include "file_system.h"
namespace latte
{

    class FileSystemWrapper: public FileSystem {
        public:
            explicit FileSystemWrapper(const std::shared_ptr<FileSystem>& t);
            ~FileSystemWrapper() override {}
            IOStatus NewRandomAccessFile(const std::string& f,
                                        const FileOptions& file_opts,
                                        std::unique_ptr<FSRandomAccessFile>* r,
                                        IODebugContext* dbg) override {
                return target_->NewRandomAccessFile(f, file_opts, r, dbg);
            }
        protected:
            std::shared_ptr<FileSystem> target_;
    };
} // namespace latte

#endif
