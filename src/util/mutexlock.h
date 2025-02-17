



#ifndef __LATTE_C_PLUS_MUTEX_LOCK_H
#define __LATTE_C_PLUS_MUTEX_LOCK_H
#include "hash.h"
namespace latte
{
    namespace rocksdb
    {
        //
        // Inspired by Guava: https://github.com/google/guava/wiki/StripedExplained
        // A striped Lock. This offers the underlying lock striping similar
        // to that of ConcurrentHashMap in a reusable form, and extends it for
        // semaphores and read-write locks. Conceptually, lock striping is the technique
        // of dividing a lock into many <i>stripes</i>, increasing the granularity of a
        // single lock and allowing independent operations to lock different stripes and
        // proceed concurrently, instead of creating contention for a single lock.
        //
        template <class T, class Key = Slice, class Hash = SliceNPHasher64>
        class Striped {
        public:
        explicit Striped(size_t stripe_count)
            : stripe_count_(stripe_count), data_(new T[stripe_count]) {}

        using Unwrapped = typename Unwrap<T>::type;
        Unwrapped &Get(const Key &key, uint64_t seed = 0) {
            size_t index = FastRangeGeneric(hash_(key, seed), stripe_count_);
            return Unwrap<T>::Go(data_[index]);
        }

        size_t ApproximateMemoryUsage() const {
            // NOTE: could use malloc_usable_size() here, but that could count unmapped
            // pages and could mess up unit test OccLockBucketsTest::CacheAligned
            return sizeof(*this) + stripe_count_ * sizeof(T);
        }

        private:
        size_t stripe_count_;
        std::unique_ptr<T[]> data_;
        Hash hash_;
        };
    } // namespace rocksdb
    
} // namespace latte



#endif