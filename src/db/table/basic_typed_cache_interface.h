


#ifndef __LATTE_C_PLUS_BASIC_TYPED_CACHE_INTERFACE_H
#define __LATTE_C_PLUS_BASIC_TYPED_CACHE_INTERFACE_H


#include "../cache/cache.h"
#include "./cache_entry_role.h"
#include "./basic_typed_cache_interface.h"

namespace latte
{
    namespace rocksdb
    {
        // BasicTypedCacheInterface - Used for primary cache storage of objects of
        // type TValue, which can be cleaned up with std::default_delete<TValue>. The
        // role is provided by TValue::kCacheEntryRole or given in an optional
        // template parameter.
        template <class TValue, CacheEntryRole kRole = TValue::kCacheEntryRole,
                typename CachePtr = Cache*>
        class BasicTypedCacheInterface : public BaseCacheInterface<CachePtr>,
                                 public BasicTypedCacheHelper<TValue, kRole> {
            public:
                CACHE_TYPE_DEFS();
                using typename BasicTypedCacheHelperFns<TValue>::TValuePtr;
                struct TypedHandle : public Handle {};
                using BasicTypedCacheHelper<TValue, kRole>::GetBasicHelper;
                // ctor
                using BaseCacheInterface<CachePtr>::BaseCacheInterface;
                struct TypedAsyncLookupHandle : public Cache::AsyncLookupHandle {
                    TypedHandle* Result() {
                        return static_cast<TypedHandle*>(Cache::AsyncLookupHandle::Result());
                    }
                };

                inline Status Insert(const Slice& key, TValuePtr value, size_t charge,
                                    TypedHandle** handle = nullptr,
                                    Priority priority = Priority::LOW) {
                    auto untyped_handle = reinterpret_cast<Handle**>(handle);
                    return this->cache_->Insert(
                        key, BasicTypedCacheHelperFns<TValue>::UpCastValue(value),
                        GetBasicHelper(), charge, untyped_handle, priority);
                }

                inline TypedHandle* Lookup(const Slice& key, Statistics* stats = nullptr) {
                    return static_cast<TypedHandle*>(this->cache_->BasicLookup(key, stats));
                }

                inline void StartAsyncLookup(TypedAsyncLookupHandle& async_handle) {
                    assert(async_handle.helper == nullptr);
                    this->cache_->StartAsyncLookup(async_handle);
                }

                inline CacheHandleGuard<TValue> Guard(TypedHandle* handle) {
                    if (handle) {
                        return CacheHandleGuard<TValue>(&*this->cache_, handle);
                    } else {
                        return {};
                    }
                }

                inline std::shared_ptr<TValue> SharedGuard(TypedHandle* handle) {
                    if (handle) {
                        return MakeSharedCacheHandleGuard<TValue>(&*this->cache_, handle);
                    } else {
                        return {};
                    }
                }

                inline TValuePtr Value(TypedHandle* handle) {
                    return BasicTypedCacheHelperFns<TValue>::DownCastValue(
                        this->cache_->Value(handle));
                }
        };
    } // namespace rocksdb
    
} // namespace latte

#endif