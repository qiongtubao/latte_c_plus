

#ifndef __LATTE_C_PLUS_UNOWNED_PTR_H
#define __LATTE_C_PLUS_UNOWNED_PTR_H


namespace latte
{
    // UnownedPtr<T> is useful as an efficient "optional reference" that can't
    // be accidentally converted to std::shared_ptr<T> nor std::unique_ptr<T>.
    template <typename T>
    class UnownedPtr {
        public:
            UnownedPtr() = default;
            UnownedPtr(std::nullptr_t) {}
            UnownedPtr(T* ptr) : ptr_(ptr) {}
            UnownedPtr(const UnownedPtr&) = default;
            UnownedPtr(UnownedPtr&&) = default;
            UnownedPtr& operator=(const UnownedPtr&) = default;
            UnownedPtr& operator=(UnownedPtr&&) = default;

            T* get() const { return ptr_; }
            T* operator->() const { return ptr_; }
            T& operator*() const { return *ptr_; }
            operator bool() const { return ptr_ != nullptr; }

        private:
            T* ptr_ = nullptr;
    };
} // namespace latte


#endif