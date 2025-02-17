
#include "version_edit.h"

namespace latte
{
    namespace leveldb
    {   
        void VersionEdit::Clear() {
            log_number_ = 0;
            prev_log_number_ = 0;
            
        }
    } // namespace level
    
} // namespace latte
