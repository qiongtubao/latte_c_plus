
#ifndef __LATTE_C_PLUS_SYSTEM_CLOCK
#define __LATTE_C_PLUS_SYSTEM_CLOCK


namespace latte
{
    // namespace rocksdb
    // {   
        //: public Customizable 
        class SystemClock {
            public:
                static const char* Type() { return "SystemClock"; }
                // 返回适合当前操作系统的默认 SystemClock。
                static const std::shared_ptr<SystemClock>& Default();
                virtual uint64_t NowMicros() = 0;
            protected:
                std::shared_ptr<SystemClock> target_;
        };
    // } // namespace rocksdb

} // namespace latte

#endif
