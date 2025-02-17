


#ifndef __LATTE_C_PLUS_BASIC_CACHE_INTERFACE_H
#define __LATTE_C_PLUS_BASIC_CACHE_INTERFACE_H


namespace latte
{
    namespace rocksdb  
    {

        
        template <typename CachePtr>
        class BaseCacheInterface {
            public:
                CACHE_TYPE_DEFS();

                /*implicit*/ BaseCacheInterface(CachePtr cache) : cache_(std::move(cache)) {}

                inline void Release(Handle* handle) { cache_->Release(handle); }

                inline void ReleaseAndEraseIfLastRef(Handle* handle) {
                    cache_->Release(handle, /*erase_if_last_ref*/ true);
                }

                inline void RegisterReleaseAsCleanup(Handle* handle, Cleanable& cleanable) {
                    cleanable.RegisterCleanup(&ReleaseCacheHandleCleanup, get(), handle);
                }

                inline Cache* get() const { return &*cache_; }

                explicit inline operator bool() const noexcept { return cache_ != nullptr; }

            protected:
                CachePtr cache_;
        };
    } // namespace rocksdb  
    
} // namespace latte



#endif