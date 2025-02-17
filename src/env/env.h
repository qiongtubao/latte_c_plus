
#ifndef __LATTE_C_PLUS_ENV_H
#define __LATTE_C_PLUS_ENV_H


#include <stdint.h>
#include <memory>
#include <cstdarg>
#include <limits>
#include <vector>
#include <functional>

#include "io/options.h"
#include "rate_limiter.h"
#include "slice/slice.h"
#include "io/file_system.h"
#include "file_lock.h"
#include <string>
#include "env_options.h"

#ifdef _WIN32
// Windows API macro interference
#undef DeleteFile
#undef GetCurrentTime
#undef LoadLibrary
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define LATTE_PRINTF_FORMAT_ATTR(format_param, dots_param) \
    __attribute__((__format__(__printf__, format_param, dots_param)))
#else
    #define LATTE_PRINTF_FORMAT_ATTR(format_param, dots_param)
#endif





namespace latte
{

    // Priority for scheduling job in thread pool
    enum Priority { BOTTOM, LOW, HIGH, USER, TOTAL };

    const size_t kDefaultPageSize = 4 * 1024;
    

    

    // namespace leveldb
    // {
    //     class Env {

    //     };
    // } // namespace leveldb
    

    // namespace rocksdb
    // {
        
    // } // namespace rocksdb
    class Env {
        public:
            // 返回适合当前操作系统的默认环境。高级用户可能希望提供自己的 Env 实现，而不是依赖于此默认环境。
            // Default() 的结果属于 rocksdb，绝不能删除。
            static Env* Default();


            // 锁定指定文件。用于防止多个进程同时访问
            // 同一个数据库。如果失败，则将 nullptr 存储在
            // *lock 中并返回非 OK。
            //
            // 如果成功，则将指向表示
            // 获取的锁的对象的指针存储在 *lock 中并返回 OK。调用者应调用
            // UnlockFile(*lock) 来释放锁。如果进程退出，
            // 锁将自动释放。
            //
            // 如果其他人已经持有锁，则立即完成
            // 并失败。即，此调用不会等待现有锁
            // 消失。
            //
            // 如果命名文件尚不存在，则可能会创建它。
            virtual Status LockFile(const std::string& fname, FileLock** lock) = 0;

            // 释放之前成功调用 LockFile 所获取的锁。
            // 要求：成功调用 LockFile() 后返回了锁
            // 要求：锁尚未解锁。
            virtual Status UnlockFile(FileLock* lock) = 0;
        public:
            enum class IOActivity : uint8_t {
                kFlush = 0,
                kCompaction = 1,
                kDBOpen = 2,
                kGet = 3,
                kMultiGet = 4,
                kDBIterator = 5,
                kVerifyDBChecksum = 6,
                kVerifyFileChecksums = 7,
                kGetEntity = 8,
                kMultiGetEntity = 9,
                kUnknown,  // Keep last for easy array of non-unknowns
            };

            enum IOPriority {
                IO_LOW = 0,
                IO_MID = 1,
                IO_HIGH = 2,
                IO_USER = 3,
                IO_TOTAL = 4
            };
            // These values match Linux definition
            // https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/fcntl.h#n56
            enum WriteLifeTimeHint {
                WLTH_NOT_SET = 0,  // No hint information set
                WLTH_NONE,         // No hints about write life time
                WLTH_SHORT,        // Data written has a short life time
                WLTH_MEDIUM,       // Data written has a medium life time
                WLTH_LONG,         // Data written has a long life time
                WLTH_EXTREME,      // Data written has an extremely long life time
            };

            virtual Status CreateDir(const std::string& dirname) = 0;
            virtual Status CreateDirIfMissing(const std::string& dirname) = 0;
            // virtual Status LockFile(const std::string& fname, FileLock** lock) = 0;
            virtual Status FileExists(const std::string& fname) = 0;
            virtual Status GetChildren(const std::string& dir,
                            std::vector<std::string>* result) = 0;
            virtual Status DeleteFile(const std::string& fname) = 0;
            virtual Status GetFileSize(const std::string& fname, uint64_t* file_size) = 0;

            const std::shared_ptr<FileSystem>& GetFileSystem() const;
            virtual std::string GenerateUniqueId();


        
    };

    // A utility routine: write "data" to the named file.
    Status WriteStringToFile(Env* env, const Slice& data, const std::string& fname,
                        bool should_sync = false,
                        const IOOptions* io_options = nullptr);

    // A utility routine: read contents of named file into *data
    Status ReadFileToString(Env* env, const std::string& fname, std::string* data);

    
} // namespace latte

#endif