#ifndef CO_H
#define CO_H

#include <coroutine>
#include <stdexcept> // 包含 std::runtime_error 所需的头文件

namespace latte {

template<typename T>
struct Task {
    struct promise_type {
        T result_value;

        // 初始化 promise 对象
        promise_type() = default;

        // 返回协程的最终结果
        auto get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // 协程开始执行前的钩子
        std::suspend_always initial_suspend() { return {}; }

        // 协程结束后的钩子
        std::suspend_never final_suspend() noexcept { return {}; }

        // 处理 co_return 语句
        void return_value(T v) {
            result_value = v;
        }

        // 注释掉或删除以下方法，因为它与返回值类型的协程不兼容
        ///*
        //void return_void() {}
        //*/

        // 异常处理
        void unhandled_exception() {
            std::terminate();
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro;

    Task(handle_type h) : coro(h) {}
    ~Task() {
        if (coro) coro.destroy();
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other) noexcept : coro(other.coro) { other.coro = nullptr; }
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            coro = other.coro;
            other.coro = nullptr;
        }
        return *this;
    }

    // 获取结果
    T get() {
        if (!coro.done()) {
            coro.resume();
        }
        if (!coro.done()) {
            throw std::runtime_error("the coroutine hasn't finished");
        }
        return coro.promise().result_value;
    }
};

} // namespace latte

#endif // CO_H