


namespace latte
{

    // namespace rocksdb
    // {
        class StaticMeta {
            public:
                

        };
        using UnrefHandler = void (*)(void* ptr);

        
        class ThreadLocalPtr {
            public:
                explicit ThreadLocalPtr(UnrefHandler handler = nullptr);
                ThreadLocalPtr(const ThreadLocalPtr&) = delete;
                ThreadLocalPtr& operator=(const ThreadLocalPtr&) = delete;
                ~ThreadLocalPtr();

                void* Get() const;

                // 初始化 ThreadLocalPtr 的静态单例。
                // 如果未调用此函数，则单例将在使用时自动初始化。
                // 两次调用此函数或在单​​例已初始化后调用此函数将无操作。
                static void InitSingletons();
            private:
                static StaticMeta* Instance();
                const uint32_t id_;
        };
    //} // namespace rocksdb
    
    
} // namespace latte
