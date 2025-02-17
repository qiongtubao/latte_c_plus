

#include "io_status.h"
#include "read_request.h"
#include "options.h"
#include "io_debug_context.h"
#include "env/env.h"

namespace latte
{
    class RandomAccessFile {
        public:
            RandomAccessFile() = default;
            
    };

    //rocksdb
    //用于随机读取文件内容的文件抽象。
    class FSRandomAccessFile {
        public:
            FSRandomAccessFile() {}
            virtual ~FSRandomAccessFile() {}

        // 从文件“offset”开始读取最多“n”个字节。
        // 此例程可以写入“scratch[0..n-1]”。将“*result”设置为已读取的数据（包括成功读取的字节数少于“n”的情况）。可以将“*result”设置为指向“scratch[0..n-1]”中的数据，因此使用“*result”时“scratch[0..n-1]”必须处于活动状态。如果遇到错误，则返回非 OK 状态。
        //
        // 调用后，仅当已到达文件末尾（或非 OK 状态）时，result->size() < n。如果在第一次 result->size() < n 之后再次调用，读取可能会失败。
        //
        // 可安全供多个线程并发使用。
        // 如果启用了直接 I/O，offset、n 和 scratch 应正确对齐。
        virtual IOStatus Read(uint64_t offset, size_t n, const IOOptions& options,
                        Slice* result, char* scratch,
                        IODebugContext* dbg) const = 0;
        
        virtual IOStatus Prefetch(uint64_t /*offset*/, size_t /*n*/,
                            const IOOptions& /*options*/,
                            IODebugContext* /*dbg*/) {
            return IOStatus::NotSupported("Prefetch");
        }

        // 按照 reqs 的描述读取一组块。这些块可以
        // 选择性地并行读取。这是一个同步调用，即它
        // 应该在所有读取完成后返回。读取将
        // 不重叠，但可以按任何顺序进行。如果函数返回状态
        // 不正常，则将忽略单个请求的状态，并假定所有读取请求都返回
        // 状态。函数返回状态
        // 仅适用于在处理单个读取
        // 请求之前发生的错误。
        virtual IOStatus MultiRead(FSReadRequest* reqs, size_t num_reqs,
                             const IOOptions& options, IODebugContext* dbg) {
            assert(reqs != nullptr);
            for (size_t i = 0; i < num_reqs; ++i) {
                FSReadRequest& req = reqs[i];
                req.status = Read(req.offset, req.len, options, &req.result, req.scratch, dbg);
            }
            return IOStatus::OK();
        }

        // 尝试获取此文件的唯一 ID，每次打开文件时该 ID 都相同（并且在文件打开期间保持不变）。
        // 此外，它会尝试使此 ID 最多为“max_size”个字节。如果可以创建这样的
        // ID，此函数将返回 ID 的长度并将其放置在“id”中；否则，此函数将返回 0，在这种情况下“id”
        // 可能未被修改。
        //
        // 此函数保证，对于给定环境中的 ID，两个唯一 ID 不能通过向其中一个添加任意字节而彼此相等。也就是说，没有一个唯一 ID 是另一个的前缀。
        //
        // 此函数保证返回的 ID 不会被解释为
        // 单个 varint。
        //
        // 注意：这些 ID 仅在进程持续时间内有效。
        virtual size_t GetUniqueId(char* /*id*/, size_t /*max_size*/) const {
            return 0;  // Default implementation to prevent issues with backwards
                    // compatibility.
        }
        enum AccessPattern { kNormal, kRandom, kSequential, kWillNeed, kWontNeed };

        virtual void Hint(AccessPattern /*pattern*/) {}

        // 向上层指示当前 RandomAccessFile 实现是否使用直接 IO。
        virtual bool use_direct_io() const { return false; }

        virtual size_t GetRequiredBufferAlignment() const { return kDefaultPageSize; }
    };
} // namespace latte
