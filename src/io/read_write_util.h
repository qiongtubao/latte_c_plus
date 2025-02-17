


#ifndef __LATTE_C_PLUS_READ_WRITE_UTIL_H
#define __LATTE_C_PLUS_READ_WRITE_UTIL_H


#include "io_status.h"
#include "file_system.h"
#include "file_options.h"

namespace latte
{
    IOStatus NewWritableFile(FileSystem* fs, const std::string& fname,
                         std::unique_ptr<FSWritableFile>* result,
                         const FileOptions& options);
    
} // namespace latte



#endif