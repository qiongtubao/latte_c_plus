

#include <string>

#include "slice.h"
#include "io_status.h"
#include <functional>

namespace latte
{
    using FSAllocationPtr = std::unique_ptr<void, std::function<void(void*)>>;

    struct FSReadRequest {
        // 表示文件偏移量的输入参数（以字节为单位）。
        uint64_t offset;
        // 输入参数，表示要读取的长度（以字节为单位）。仅限 `result`
        // 如果到达文件末尾（或 `status` 不正常），则返回较少的字节。
        size_t len;

        // MultiRead() 可以选择将数据放入的缓冲区。它可以
        // 忽略此操作并分配自己的缓冲区。
        // 暂存的生命周期将持续到 IO 完成。
        //
        // 在异步读取的情况下，它是一个输出参数，它将
        // 保持到调用回调为止。暂存由 RocksDB 分配
        // 并将传递给底层文件系统。
        char* scratch;

        // MultiRead() 设置的输出参数指向数据缓冲区，以及
        // 有效字节数
        //
        // 如果是异步读取，此输出参数由 Async Read
        // API 设置，指向数据缓冲区，以及
        // 有效字节数。
        // 切片结果应指向暂存区，即数据应
        // 始终读入暂存区。
        Slice result;
        // 由底层文件系统设置的输出参数，表示读取请求的状态。
        IOStatus status;
        // fs_scratch 是底层 FileSystem 在读取期间分配并提供给 RocksDB 的数据缓冲区，当 FS 想要为自己的缓冲区提供数据时，而不是使用 RocksDB 提供的 FSReadRequest::scratch。
        //
        // FileSystem 需要提供缓冲区和自定义删除函数。fs_scratch 的生命周期直到 RocksDB 使用数据。缓冲区应该由 RocksDB 使用 unique_ptr fs_scratch 中提供的自定义删除函数释放。
        //
        // 优化优势：
        // 这在底层 FileSystem 必须将数据额外复制到 RocksDB 提供的缓冲区（这会消耗 CPU 周期）的情况下很有用。可以通过避免复制到 RocksDB 缓冲区并直接使用 FS 提供的缓冲区来进行优化。
        //
        // 如何启用：
        // 为了启用此选项，FS 需要重写 SupportedOps() API 并
        // 在 SupportedOps() 中将 FSSupportedOps::kFSBuffer 设置为：
        // {
        // supports_ops |= (1 << FSSupportedOps::kFSBuffer);
        // }
        //
        // 正在进行中：
        // 目前，它仅适用于非直接 io 的 MultiReads（同步和异步
        // 两者）。
        // 如果 RocksDB 在读取期间提供自己的缓冲区（scratch），则这是一个
        // 信号，让 FS 使用 RocksDB 缓冲区。
        // 如果启用了 FSSupportedOps::kFSBuffer 并且 scratch == nullptr，
        // 那么 FS 必须在 fs_scratch 中提供自己的缓冲区。
        //
        // 注意：
        // - FSReadRequest::result 应该指向 fs_scratch。
        // - 仅当底层 FS 提供 FSSupportedOps::kFSBuffer 支持时才需要此功能。
        FSAllocationPtr fs_scratch;

        
    };
} // namespace latte
