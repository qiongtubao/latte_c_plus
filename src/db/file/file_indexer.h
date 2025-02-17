



#ifndef __LATTE_C_PLUS_FILE_INDEXER_H
#define __LATTE_C_PLUS_FILE_INDEXER_H

#include "comparator/comparator.h"
namespace latte
{
    class FileIndexer {
        public:
            explicit FileIndexer(const Comparator* ucmp);
    };
} // namespace latte


#endif