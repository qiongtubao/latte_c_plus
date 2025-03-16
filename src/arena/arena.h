#include "allocator.h"
#include "alloc_tracker.h"
#include <deque>
#include <cstdint>
#include <memory>
#include <string>
#include <cstddef>
#include <vector>
#include <cassert>

namespace latte
{
    class Arena {
        public:
            Arena();
            Arena(const Arena&) = delete;
            Arena& operator=(const Arena&) = delete;

            ~Arena();

            // Return a pointer to a newly allocated memory block of "bytes" bytes.
            char* Allocate(size_t bytes);

            // Allocate memory with the normal alignment guarantees provided by malloc.
            char* AllocateAligned(size_t bytes);

            // Returns an estimate of the total memory usage of data allocated
            // by the arena.
            size_t MemoryUsage() const {
                return memory_usage_.load(std::memory_order_relaxed);
            }

            private:
            char* AllocateFallback(size_t bytes);
            char* AllocateNewBlock(size_t block_bytes);

            // Allocation state
            char* alloc_ptr_;
            size_t alloc_bytes_remaining_;

            // Array of new[] allocated memory blocks
            std::vector<char*> blocks_;

            // Total memory usage of the arena.
            //
            // TODO(costan): This member is accessed via atomics, but the others are
            //               accessed without any locking. Is this OK?
            std::atomic<size_t> memory_usage_;
    };

    inline char* Arena::Allocate(size_t bytes) {
        // The semantics of what to return are a bit messy if we allow
        // 0-byte allocations, so we disallow them here (we don't need
        // them for our internal use).
        // 如果我们允许 0 字节分配，则返回内容的语义会有点混乱，
        // 因此我们在此禁止它们（我们不需要它们供内部使用）。
        assert(bytes > 0);
        //大小小于剩余字节数
        if (bytes <= alloc_bytes_remaining_) {
            //记录当前最后指针
            char* result = alloc_ptr_;
            //调整指针最后位置
            alloc_ptr_ += bytes;
            //调整剩余字节数
            alloc_bytes_remaining_ -= bytes;
            return result;
        }
        return AllocateFallback(bytes);
    }
    // class Arena : public Allocator {
    //     public:
    //         Arena(const Arena&) = delete;
    //         void operator=(const Arena&) = delete;

    //         static constexpr size_t kInlineSize = 2048;
    //         static constexpr size_t kMinBlockSize = 4096;
    //         static constexpr size_t kMaxBlockSize = 2u << 30;
    //         static constexpr unsigned kAlignUnit = alignof(std::max_align_t);
    //         static_assert((kAlignUnit & (kAlignUnit - 1)) == 0,
    //             "Pointer size should be power of 2");
            
    //         // huge_page_size：如果为 0，则不使用大页面 TLB。
    //         // 如果 > 0（应设置为系统支持的大页面大小），
    //         // 块分配将首先尝试大页面 TLB。如果分配失败，将恢复到正常情况。
    //         explicit Arena(size_t block_size = kMinBlockSize, 
    //             AllocTracker* tracker = nullptr, size_t huge_page_size = 0);
            
    //         ~Arena();

    //         char* Allocate(size_t bytes) override;

    //         // huge_page_size：如果 >0，将尝试从 huage 页面 TLB 分配。
    //         // 参数将是超大页面 TLB 的页面大小。字节
    //         // 将四舍五入为页面大小的倍数，以通过 mmap
    //         // 启用超大页面的匿名选项进行分配。分配的额外空间将被
    //         // 浪费。如果分配失败，将恢复到正常情况。要启用它，
    //         // 需要保留超大页面以供分配，例如：
    //         // sysctl -w vm.nr_hugepages=20
    //         // 有关详细信息，请参阅 linux doc Documentation/vm/hugetlbpage.txt。
    //         // 超大页面分配可能会失败。在这种情况下，它将恢复到
    //         // 正常情况。消息将记录到记录器中。因此，当使用
    //         // huge_page_tlb_size > 0 进行调用时，我们强烈建议传入一个记录器。
    //         // 否则，错误消息将直接打印到 stderr。
    //         char* AllocateAligned(size_t bytes, size_t huge_page_size = 0) override;

    //         // 返回竞技场分配的数据的总内存使用量的估计值（不包括已分配但尚未用于未来分配的空间）。
    //         size_t ApproximateMemoryUsage() const;

    //         size_t MemoryAllocatedBytes() const;

    //         size_t AllocatedAndUnused() const;

    //         // 如果分配太大，我们将分配一个与该分配大小相同的不规则块。
    //         size_t IrregularBlockNum() const;

    //         size_t BlockSize() const override;

    //         bool IsInInlineBlock() const;

    //         // 检查并调整 block_size，使得返回值为
    //         // 1. 在 [kMinBlockSize, kMaxBlockSize] 范围内。
    //         // 2. 对齐单元的倍数。
    //         static size_t OptimizeBlockSize(size_t block_size);

    //     private:
    //         alignas(std::max_align_t) char inline_block_[kInlineSize];
    //         // 一个块中分配的字节数
    //         const size_t kBlockSize;
    //         // 分配的内存块
    //         std::deque<std::unique_ptr<char[]>> blocks_;
    //         size_t irregualr_block_num = 0;
    //         // 当前活动块的统计信息。
    //         // 对于每个块，我们从一端分配对齐的内存块，并从另一端分配未对齐的内存块。否则，如果我们从一个方向分配两种类型的内存，则对齐的内存浪费会更高。
    //         char* unaligned_alloc_ptr_ = nullptr;
    //         char* aligned_alloc_ptr_ = nullptr; 
    //         // 当前活动块中还剩下多少字节？
    //         size_t alloc_bytes_remaining_ = 0;
    //         size_t hugetlb_size_ = 0;

    //         char* AllocateFromHugePage(size_t bytes);
    //         char* AllocateFallback(size_t bytes, bool aligned);
    //         char* AllocateNewBlock(size_t block_bytes);

    //         // 到目前为止已分配的块中的内存字节数
    //         size_t blocks_memory_ = 0;
    //         // 非自有
    //         AllocTracker* tracker_;

    // };
} // namespace latte
