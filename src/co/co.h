#include <coroutine>
#include <exception>

namespace latte {

template<typename T>
struct Task {
    struct promise_type;

    struct awaiter {
        std::coroutine_handle<promise_type> coro_;

        bool await_ready() const noexcept { return !coro_ || coro_.done(); }
        void await_suspend(std::coroutine_handle<> h) const noexcept { coro_.promise().continuation = h; }
        decltype(auto) await_resume() { 
            if (coro_.promise().exception) {
                std::rethrow_exception(coro_.promise().exception);
            }
            return coro_.promise().result_value;
        }

        explicit awaiter(std::coroutine_handle<promise_type> h) : coro_(h) {}
    };

    struct promise_type {
        T result_value;
        std::exception_ptr exception;
        std::coroutine_handle<> continuation{nullptr};

        auto get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }

        void return_value(T value) { result_value = value; }
        void unhandled_exception() { exception = std::current_exception(); }

        awaiter operator co_await() const & { return awaiter{std::coroutine_handle<promise_type>::from_promise(*this)}; }
    };

    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro_;

    Task(handle_type h) : coro_(h) {}
    ~Task() { if (coro_) coro_.destroy(); }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other) noexcept : coro_(other.coro_) { other.coro_ = nullptr; }
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (coro_) coro_.destroy();
            coro_ = other.coro_;
            other.coro_ = nullptr;
        }
        return *this;
    }

    T get() {
        while (!coro_.done()) {
            coro_.resume();
        }
        if (coro_.promise().exception) {
            std::rethrow_exception(coro_.promise().exception);
        }
        return coro_.promise().result_value;
    }
};

// Specialization for Task<void>
template<>
struct Task<void> {
    struct promise_type;

    struct awaiter {
        std::coroutine_handle<promise_type> coro_;

        bool await_ready() const noexcept { return !coro_ || coro_.done(); }
        void await_suspend(std::coroutine_handle<> h) const noexcept { coro_.promise().continuation = h; }
        void await_resume() { 
            if (coro_.promise().exception) {
                std::rethrow_exception(coro_.promise().exception);
            }
        }

        explicit awaiter(std::coroutine_handle<promise_type> h) : coro_(h) {}
    };

    struct promise_type {
        std::exception_ptr exception;
        std::coroutine_handle<> continuation{nullptr};

        auto get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }

        void return_void() {}
        void unhandled_exception() { exception = std::current_exception(); }

        awaiter operator co_await() const & { return awaiter{std::coroutine_handle<promise_type>::from_promise(*this)}; }
    };

    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro_;

    Task(handle_type h) : coro_(h) {}
    ~Task() { if (coro_) coro_.destroy(); }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other) noexcept : coro_(other.coro_) { other.coro_ = nullptr; }
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (coro_) coro_.destroy();
            coro_ = other.coro_;
            other.coro_ = nullptr;
        }
        return *this;
    }

    void get() {
        while (!coro_.done()) {
            coro_.resume();
        }
        if (coro_.promise().exception) {
            std::rethrow_exception(coro_.promise().exception);
        }
    }
};
}