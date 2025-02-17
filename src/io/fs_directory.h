

#ifndef __LATTE_C_PLUS_IO_FS_DIRECTORY_H
#define __LATTE_C_PLUS_IO_FS_DIRECTORY_H

namespace latte
{
    
    class FSDirectory {
        public:
            // For cases when Close() hasn't been called, many derived classes of
            // FSDirectory will need to call Close() non-virtually in their destructor,
            // and ignore the result, to ensure resources are released.
            virtual ~FSDirectory() {}
            
    };
    
} // namespace latte



#endif