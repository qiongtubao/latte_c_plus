


#ifndef __LATTE_C_PLUS_WRITE_STATE_H
#define __LATTE_C_PLUS_WRITE_STATE_H
#include <string>
namespace latte
{
    namespace rocksdb
    {
        enum Write_State: uint8_t {
            // 写入器的初始状态。这是在 JoinBatchGroup 中等待的写入器。
            // 当另一个线程通知等待者它已成为组长时，可以离开此状态（-> STATE_GROUP_LEADER），
            // 当选择非并行的领导者通知跟随者其写入已提交时（-> STATE_COMPLETED），
            // 或者当选择并行执行更新并需要此写入器应用其批处理时（-> STATE_PARALLEL_MEMTABLE_WRITER）。
            STATE_INIT = 1,

            // 该状态用于通知等待的 Writer 它已成为
            // 领导者，现在它应该构建一个写入批处理组。棘手：
            // 如果在 Writer 将自身入队时 newest_writer_ 为空，则不使用此状态，因为在这种情况下无需等待（甚至无需
            // 创建用于等待的互斥锁和 condvar）。这是
            // 终端状态，除非领导者选择将其设为并行
            // 批处理，在这种情况下，最后一个完成的并行工作器将使
            // 领导者进入 STATE_COMPLETED。
            STATE_GROUP_LEADER = 2,

            // 用于通知等待写入器它已成为
            // memtable 写入器组的领导者的状态。领导者将为整个组写入
            // memtable，或者通过调用 LaunchParallelMemTableWrite 启动对 memtable 的并行组写入
            //。
            STATE_MEMTABLE_WRITER_LEADER = 4,

            // 用于通知等待写入器它已成为
            // 并行内存表写入器的状态。它可以是启动
            // 并行写入器组的组长，也可以是跟随者之一。然后，写入器应
            // 将其批次同时应用于内存表并调用
            // CompleteParallelMemTableWriter。
            STATE_PARALLEL_MEMTABLE_WRITER = 8,

            // 追随者的写入操作已完成，或并行领导者
            // 其追随者已全部完成其工作。这是终端
            // 状态。
            STATE_COMPLETED = 16,

            // 使用 StateMutex() 表示线程可能正在等待的状态
            // 和 StateCondVar()
            STATE_LOCKED_WAITING = 32,

            // 用于通知等待写入器它已成为
            // 调用者的状态，通过调用 SetMemWritersEachStride 来调用其他等待写入器写入 memtable
            //。执行此操作后
            // 它还将写入 memtable。
            STATE_PARALLEL_MEMTABLE_CALLER = 64,
        };
    } // namespace rocksdb
    
} // namespace latte


#endif