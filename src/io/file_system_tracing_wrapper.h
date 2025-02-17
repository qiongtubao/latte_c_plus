



#ifndef __LATTE_C_PLUS_ENV_FILE_SYSTEM_TRACING_WRAPPER_H
#define __LATTE_C_PLUS_ENV_FILE_SYSTEM_TRACING_WRAPPER_H

#include "file_system_wrapper.h"
namespace latte
{
    // namespace rocksdb
    // {
        class FileSystemTracingWrapper : public FileSystemWrapper {
            public:
                IOStatus NewRandomAccessFile(const std::string& fname,
                               const FileOptions& file_opts,
                               std::unique_ptr<FSRandomAccessFile>* result,
                               IODebugContext* dbg) override;
            
        };
    // } // namespace rocksdb
    
} // namespace latte


#endif