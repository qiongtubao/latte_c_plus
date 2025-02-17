





#ifndef __LATTE_C_PLUS_THREAD_POOL_IMPL_H
#define __LATTE_C_PLUS_THREAD_POOL_IMPL_H

#include "thread_pool.h"
#include <memory>
#include <functional>
#include "env/env.h"
#include "port/port_defs.h"

namespace latte
{
    class ThreadPoolImpl : public ThreadPool {
        public:
            ThreadPoolImpl();
            ~ThreadPoolImpl();

            ThreadPoolImpl(ThreadPoolImpl&&) = delete;
            ThreadPoolImpl& operator=(ThreadPoolImpl&&) = delete;

            // 实现 ThreadPool 接口

            // 等待所有线程完成。
            // 丢弃所有尚未开始执行的作业，并等待正在运行的作业
            // 完成
            void JoinAllThreads() override;

            // 设置将执行计划作业的后台线程数。
            void SetBackgroundThreads(int num) override;
            int GetBackgroundThreads() override;

            // 获取 ThreadPool 队列中安排的作业数。
            unsigned int GetQueueLen() const override;

            // 等待所有作业完成
            // 已经开始运行的作业和
            // 尚未开始运行的作业
            void WaitForJobsAndJoinAllThreads() override;

            // 使线程以较低的内核 IO 优先级运行
            // 目前仅对 Linux 有效
            void LowerIOPriority();

            // 使线程以较低的内核 CPU 优先级运行
            // 目前仅对 Linux 有效
            void LowerCPUPriority(CpuPriority pri);

            // 确保池中至少有 num 个线程
            // 但如果有更多的线程，则不要终止线程
            void IncBackgroundThreadsIfNeeded(int num);

            // 提交一个触发后就不管的作业
            // 这些作业不能取消计划

            // 这允许多次提交同一项作业
            void SubmitJob(const std::function<void()>&) override;
            // 这会将函数移入以提高效率
            void SubmitJob(std::function<void()>&&) override;

            // 使用取消调度标签和取消调度函数来调度作业
            // 可用于通过标签筛选和取消调度仍在队列中且尚未开始运行的作业
            //
            void Schedule(void (*function)(void* arg1), void* arg, void* tag,
                            void (*unschedFunction)(void* arg));

            // 过滤仍在队列中的作业并匹配
            // 给定的标签。如果有的话，将其从队列中移除
            // 并且对每个这样的作业执行取消调度函数
            // 如果在调度时给出了这样的函数。
            int UnSchedule(void* tag);

            void SetHostEnv(Env* env);

            Env* GetHostEnv() const;

            // 返回线程优先级。
            // 这将允许其成员线程知道其优先级。
            Priority GetThreadPriority() const;

            // 设置线程优先级。
            void SetThreadPriority(Priority priority);

            // 保留特定数量的线程，防止它们运行其他
            // 函数保留的线程数可能少于所需的
            // 一个
            int ReserveThreads(int threads_to_be_reserved) override;

            // 释放特定数量的线程
            int ReleaseThreads(int threads_to_be_released) override;

            static void PthreadCall(const char* label, int result);

        struct Impl;

        private:
            // 当前公共接口不提供可用功能，因此无法在内部使用以展现不同的实现。
            //
            // 我们提出了一种 pimpl 习惯用法，以便轻松替换线程池实现，而无需触及头文件，但可能提供不同的 .cc，由 CMake 选项驱动。
            //
            // 另一种选择是引入 Env::MakeThreadPool() 虚拟接口并覆盖环境。这需要重构 ThreadPool 的使用。
            //
            // 我们还可以结合这两种方法
            std::unique_ptr<Impl> impl_;
    };
} // namespace latte




#endif