




#ifndef __LATTE_C_PLUS_THREAD_POOL_H
#define __LATTE_C_PLUS_THREAD_POOL_H

#include <functional>

namespace latte
{
    class ThreadPool {
        public:
            virtual ~ThreadPool() {}

        // 等待所有线程完成。
        // 丢弃那些未启动的线程
        // 正在执行
        virtual void JoinAllThreads() = 0;

        // 设置将执行
        // 计划作业的后台线程数。
        virtual void SetBackgroundThreads(int num) = 0;
        virtual int GetBackgroundThreads() = 0;

        // 获取 ThreadPool 队列中安排的作业数。
        virtual unsigned int GetQueueLen() const = 0;

        // 等待所有作业完成（那些已经开始运行的作业和那些尚未开始运行的作业）。这可确保在 TP 上抛出的所有内容都能运行，即使我们可能没有为作业数量指定足够的线程
        virtual void WaitForJobsAndJoinAllThreads() = 0;

        // 提交一个触发后就不管的作业
        // 这允许多次提交相同的作业
        virtual void SubmitJob(const std::function<void()>&) = 0;
        // 这会将函数移入以提高效率
        virtual void SubmitJob(std::function<void()>&&) = 0;

        // 保留可用的后台线程。此函数不确保
        // 可以保留这么多线程，而是返回可以保留的线程数（相对于所需线程数）。换句话说，
        // 可用线程数可能小于输入。
        virtual int ReserveThreads(int /*threads_to_be_reserved*/) { return 0; }

        // 释放特定数量的保留线程
        virtual int ReleaseThreads(int /*threads_to_be_released*/) { return 0; }    
    };
} // namespace latte




#endif