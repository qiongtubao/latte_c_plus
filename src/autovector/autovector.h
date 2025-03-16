#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace latte
{
    template <class TAutoVector, class TValueType>
    class iterator_impl {
        public:
            // -- iterator traits
            using self_type = iterator_impl<TAutoVector, TValueType>;
            using value_type = TValueType;
            using reference = TValueType&;
            using pointer = TValueType*;
            using difference_type = typename TAutoVector::difference_type;
            using iterator_category = std::random_access_iterator_tag;

            iterator_impl(TAutoVector* vect, size_t index):
                vect_(vect), index_(index) {}
            iterator_impl(const iterator_impl&) = default;
            ~iterator_impl() {}
            iterator_impl& operator=(const iterator_impl&) = default;

            // 重构 ++iterator
            self_type& operator++() {
                ++index_;
                return *this;
            }

            //重构 iterator++
            self_type operator++(int) {
                auto old = *this;
                ++index_;
                return old;
            }

            //重构 --iterator
            self_type& operator--() {
                --index_;
                return *this;
            }

            //重构 iterator--
            self_type& operator--(int) {
                auto old = *this;
                --index_;
                return old;
            }

            self_type operator-(difference_type len) const {
                return self_type(vect_, index_ - len);
            }

            difference_type operator-(const self_type& other) const {
                assert(vect_ == other.vect_);
                return index_ - other.index_;
            }

            self_type operator+(difference_type len) const {
                return self_type(vect_, index_ + len);
            }

            self_type& operator+=(difference_type len) {
                index_ += len;
                return *this;
            }

            self_type& operator-=(difference_type len) {
                index_ -= len;
                return *this;
            }

            // -- Reference
            reference operator*() const {
                assert(vect_->size() >= index_);
                return (*vect_)[index_];
            }

            pointer operator->() const {
                assert(vect_->size() >= index_);
                return &(*vect_)[index_];
            }

            reference operator[](difference_type len) const { return *(*this + len); }

            // -- Logical Operators
            bool operator==(const self_type& other) const {
                assert(vect_ == other.vect_);
                return index_ == other.index_;
            }

            bool operator!=(const self_type& other) const { return !(*this == other); }

            bool operator>(const self_type& other) const {
                assert(vect_ == other.vect_);
                return index_ > other.index_;
            }

            bool operator<(const self_type& other) const {
                assert(vect_ == other.vect_);
                return index_ < other.index_;
            }

            bool operator>=(const self_type& other) const {
                assert(vect_ == other.vect_);
                return index_ >= other.index_;
            }

            bool operator<=(const self_type& other) const {
                assert(vect_ == other.vect_);
                return index_ <= other.index_;
            }
        private:
            TAutoVector* vect_ = nullptr;
            size_t index_ = 0;
     
    };
    template <class T, size_t kSize = 8>
    class autovector {
        public:
            using value_type = T;
            using difference_type = typename std::vector<T>::difference_type;
            using size_type = typename std::vector<T>::size_type;
            using reference = value_type&;
            using const_reference = const value_type&;
            using pointer = value_type*;
            using const_pointer = const value_type*;

            //定义迭代器
            using iterator = iterator_impl<autovector, value_type>;
            using const_iterator = iterator_impl<const autovector, const value_type>;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            autovector() : values_(reinterpret_cast<pointer>(buf_)) {}

            autovector(std::initializer_list<T> init_list)
                : values_(reinterpret_cast<pointer>(buf_)) {   
                for (const T& item: init_list) {
                    push_back(item);
                }
            }

            ~autovector() {
                clear();
            }

            // 返回指向容器中第一个元素的迭代器。
            iterator begin() { return iterator(this, 0); }
            const_iterator begin() const { return const_iterator(this, 0); }

            //返回指向容器最后一个元素之后位置的迭代器。
            iterator end() { return iterator(this, this->size()); }
            const_iterator end() const { return const_iterator(this, this->size()); }

            // 返回容器中的元素数量。
            size_type size() {
                return num_stack_items_ + vect_.size();
            }
            // 返回当前分配给 vector 的存储空间可以容纳的元素个数。
            size_type capacity() const {
                return kSize + vect_.capacity();
            }
            // 判断容器是否为空（无元素），若为空则返回 true。
            bool empty() const {
                return size() == 0;
            }

            // 请求容器容量至少能够容纳 count 个元素
            void reserve(size_t cap) {
                if (cap > kSize) {
                    vect_.reserve(cap - kSize);
                }
                assert(cap <= capacity());
            }

            // 改变容器中的元素数量为指定的数量。如果新的尺寸更大，新添加的元素将使用默认值或提供的值进行初始化。
            void resize(size_type n) {
                if (n > kSize) {
                    vect_.resize(n - kSize);
                    while (num_stack_items_ < kSize) {
                        new ((void*)(&values_[num_stack_items_++])) value_type();
                    }
                     num_stack_items_ = kSize;
                } else {
                    vect_.clear();
                    while (num_stack_items_ < n) {
                        new ((void*)(&values_[num_stack_items_++])) value_type();
                    }
                    while (num_stack_items_ > n) {
                        values_[--num_stack_items_].~value_type();
                    }
                }
            }

            // -- Mutable Operations
            // 在容器的末尾添加一个元素。
            void push_back(T&& item) {
                if (num_stack_items_ < kSize) {
                    new ((void*)(&values_[num_stack_items_])) value_type();
                    values_[num_stack_items_++] = std::move(item);
                } else {
                    vect_.push_back(item);
                }
            }
            void push_back(const T& item) {
                if (num_stack_items_ < kSize) {
                    new ((void*)(&values_[num_stack_items_])) value_type();
                    values_[num_stack_items_++] = item;
                } else {
                    vect_.push_back(item);
                }
            }
            // 清除容器中的所有元素，使其变为空容器。
            void clear() {
                while (num_stack_items_ > 0) {
                    values_[--num_stack_items_].~value_type();
                }
                vect_.clear();
            }
            // 移除容器中的最后一个元素。
            void pop_back() {
                assert(!empty());
                if (!vect_.empty()) {
                    vect_.pop_back();
                } else {
                    values_[--num_stack_items_].~value_type();
                }
            }
            // 通过索引访问元素，并提供边界检查。如果索引越界，则会抛出异常。
            const_reference at(size_type n) const {
                assert(n < size());
                return (*this)[n];
            }
            reference at(size_type n) {
                assert(n < size());
                return (*this)[n];
            }
            //返回容器中第一个元素的引用。
            reference front() {
                assert(!empty());
                return *begin();
            }
            const_reference front() const {
                assert(!empty());
                return *begin();
            }
            // 返回容器中最后一个元素的引用。
            reference back() {
                assert(!empty());
                return *(end() - 1);
            }
            const_reference back() const {
                assert(!empty());
                return *(end() - 1);
            }
            // insert();
            // erase();
            // data();

            const_reference operator[](size_type n) const {
                assert(n < size());
                if (n < kSize) {
                    return values_[n];
                }
                return vect_[n - kSize];
            }

            reference operator[](size_type n) {
                assert(n < size());
                if (n < kSize) {
                    return values_[n];
                }
                return vect_[n - kSize];
            }

            
            
            
            


            private:
                size_type num_stack_items_ = 0;  // current number of itemss
                alignas(alignof(
                    value_type)) char buf_[kSize *
                                            sizeof(value_type)];  // the first `kSize` items
                pointer values_;
                // used only if there are more than `kSize` items.
                std::vector<T> vect_;


    };
} // namespace latte
