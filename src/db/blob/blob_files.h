



#ifndef __LATTE_C_PLUS_BLOB_FILES_H
#define __LATTE_C_PLUS_BLOB_FILES_H

#include <vector>
#include "./blob_file_meta_data.h"
#include <memory>

namespace latte
{
    
    using BlobFiles = std::vector<std::shared_ptr<BlobFileMetaData>>;

    
} // namespace latte



#endif