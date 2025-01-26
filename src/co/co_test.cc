#include <iostream>
#include <chrono>
#include <coroutine>

struct Task {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        // 返回最终的对象
        Task get_return_object() { 
            return Task{handle_type::from_promise(*this)}; 
        }
        // 初始挂起点
        std::suspend_never initial_suspend() { return {}; }
        // 最终挂起点
        std::suspend_never final_suspend() noexcept { return {}; }
        // 处理未捕获的异常
        void unhandled_exception() { std::terminate(); }
        // 返回 void
        void return_void() {}

        auto yield_value(int value) {
            this->value = value;
            return std::suspend_always{};
        }

        int value;
    };

    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        // 模拟异步操作完成后恢复协程
        std::thread([this, h]() mutable {
            // 模拟耗时操作
            std::this_thread::sleep_for(std::chrono::seconds(2));
            // 恢复协程
            h.resume();
        }).detach();
    }
    int await_resume() { return handle.promise().value; }

    handle_type handle;

    Task(handle_type h) : handle(h) {}
    ~Task() {
        if (handle)
            handle.destroy();
    }
};

Task async_task(int value) {
    std::cout << "Starting async task with value: " << value << std::endl;
    co_await std::suspend_always{}; // 模拟异步等待
    std::cout << "Async task finished" << std::endl;
    co_return value * 2; // 返回处理后的值
}

int main() {
    // 启动异步任务
    auto task = async_task(10);
    
    std::cout << "Main thread is doing other work..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3)); // 主线程做其他工作

    // 获取异步任务的结果
    if (!task.handle.done()) {
        task.handle.resume(); // 如果任务还没完成，手动恢复
    }
    std::cout << "Result from async task: " << task.await_resume() << std::endl;

    return 0;
}