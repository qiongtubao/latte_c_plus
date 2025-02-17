


#ifndef __LATTE_C_PLUS_ENV_OPTIONS_H
#define __LATTE_C_PLUS_ENV_OPTIONS_H

#include <stdint.h>
#include <string>
#include "rate_limiter.h"

namespace latte
{
    struct EnvOptions {
        // 使用默认选项构造
        EnvOptions();

        // 根据选项构造
        // explicit EnvOptions(const DBOptions& options);

        // 如果为真，则使用 mmap 读取数据。
        // 不推荐用于 32 位操作系统。
        bool use_mmap_reads = false;

        // 如果为 true，则使用 mmap 写入数据
        bool use_mmap_writes = true;

        // 如果为真，则使用 O_DIRECT 读取数据
        bool use_direct_reads = false;

        // 如果为真，则使用 O_DIRECT 写入数据
        bool use_direct_writes = false;

        // 如果为 false，则绕过 fallocate() 调用
        bool allow_fallocate = true;

        // 如果为真，则在打开的 fd 上设置 FD_CLOEXEC。
        bool set_fd_cloexec = true;

        // 允许操作系统在后台将文件增量同步到磁盘，同时写入文件。每次写入 bytes_per_sync 时发出一个请求。0 将其关闭。
        // 默认值：0
        uint64_t bytes_per_sync = 0;

        // 为 true 时，保证文件在任何给定时间最多提交 `bytes_per_sync` 个字节用于写回。
        //
        // - 如果支持 `sync_file_range`，则通过等待任何
        // 先前的 `sync_file_range` 完成之后再继续来实现此目的。这样，
        // 处理（压缩等）可以在 `sync_file_range` 之间的间隙中不受阻碍地进行，并且我们仅在 I/O 落后时阻塞。
        // - 否则使用 `WritableFile::Sync` 方法。请注意，此机制
        // 始终阻塞，从而防止 I/O 和处理交错。
        //
        // 注意：启用此选项不提供任何额外的持久性
        // 保证，因为它可能使用 `sync_file_range`，而它不会写出
        // 元数据。
        //
        // 默认值：false
        bool strict_bytes_per_sync = false;

        // 如果为 true，我们将使用 FALLOC_FL_KEEP_SIZE 标志预分配文件，这意味着文件大小不会随着预分配而改变。
        // 如果为 false，预分配也会改变文件大小。此选项将提高在每次写入时同步数据的工作负载的性能。默认情况下，我们将其设置为 true 用于 MANIFEST 写入，false 用于
        // WAL 写入
        bool fallocate_with_keep_size = true;

        // 参见 DBOptions 文档
        size_t compaction_readahead_size = 0;

        // 参见 DBOptions 文档
        size_t random_access_max_buffer_size = 0;

        // 参见 DBOptions 文档
        size_t writable_file_max_buffer_size = 1024 * 1024;

        // 如果不是 nullptr，则启用刷新和压缩的写入速率限制
        RateLimiter* rate_limiter = nullptr;
    };
} // namespace latte


#endif