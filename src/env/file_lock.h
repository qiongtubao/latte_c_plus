

#ifndef __LATTE_C_PLUS_ENV_FILE_LOCK_H
#define __LATTE_C_PLUS_ENV_FILE_LOCK_H

namespace latte
{
    
    class FileLock {
        public:
            FileLock() = default;

            FileLock(const FileLock&) = delete;
            FileLock& operator=(const FileLock&) = delete;

            virtual ~FileLock();
    };

    
} // namespace latte

#endif

