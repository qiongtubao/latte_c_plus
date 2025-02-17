

#include <string>
#include <status/status.h>

namespace latte
{
    class RateLimiter {
        public:
            enum class OpType {
                kRead,
                kWrite,
            };

            enum class Mode {
                kReadsOnly = 0,
                kWritesOnly = 1,
                kAllIo = 2,
            };

            // 为了 API 兼容性，默认仅限制写入速率。
            explicit RateLimiter(Mode mode = Mode::kWritesOnly) : mode_(mode) {}

            virtual ~RateLimiter() {}

            // 此 API 允许用户动态更改速率限制器的每秒字节数。
            // 必需：bytes_per_second > 0
            virtual void SetBytesPerSecond(int64_t bytes_per_second) = 0;

            // 此 API 允许用户动态更改在一次 `Request()` 调用中可以授予的最大字节数。零是一个特殊值，表示每次重新填充的字节数。
            //
            // 必需：single_burst_bytes >= 0。否则将返回 `Status::InvalidArgument`。
            virtual Status SetSingleBurstBytes(int64_t /* single_burst_bytes */) {
                return Status::NotSupported();
            }

            protected:
                Mode GetMode() { return mode_; }
            private:
                const Mode mode_;
    };
} // namespace latte
