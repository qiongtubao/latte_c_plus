

#ifndef __LATTE_C_PLUS_LOGGING_H
#define __LATTE_C_PLUS_LOGGING_H


#include <functional>

namespace latte
{

    namespace log
    {
        enum InfoLogLevel : unsigned char {
            DEBUG_LEVEL = 0,
            INFO_LEVEL,
            WARN_LEVEL,
            ERROR_LEVEL,
            FATAL_LEVEL,
            HEADER_LEVEL,
            NUM_INFO_LOG_LEVELS,
        };
        class Logger {
            public:
                explicit Logger(const InfoLogLevel log_level = InfoLogLevel::INFO_LEVEL)
                    : closed_(false), log_level_(log_level) {}
                virtual InfoLogLevel GetInfoLogLevel() const { return log_level_; }
                virtual void Logv(const InfoLogLevel log_level, const char* format,
                    va_list ap);
                    
                // Flush to the OS buffers    
                virtual void Flush() {}
            protected:
                virtual Status CloseImpl();
                bool closed_;
            private:
                InfoLogLevel log_level_;

        };
        static void Logv(Logger* info_log, const char* format, va_list ap) {
            if (info_log && info_log->GetInfoLogLevel() <= InfoLogLevel::INFO_LEVEL) {
                info_log->Logv(InfoLogLevel::INFO_LEVEL, format, ap);
            }
        };
        #if defined(__GNUC__) || defined(__clang__)
        #define ROCKSDB_PRINTF_FORMAT_ATTR(format_param, dots_param) \
            __attribute__((__format__(__printf__, format_param, dots_param)))
        #else
            #define ROCKSDB_PRINTF_FORMAT_ATTR(format_param, dots_param)
        #endif
        // The default info log level is InfoLogLevel::INFO_LEVEL.
        void Log(const InfoLogLevel log_level, const std::shared_ptr<Logger>& info_log,
         const char* format, ...);
        void LogFlush(Logger* info_log) {
            if (info_log) {
                info_log->Flush();
            }
        }
        void LogFlush(const std::shared_ptr<Logger>& info_log) {
            LogFlush(info_log.get());
        }
        // void Log(Logger* info_log, const char* format, ...) {
        //     va_list ap;
        //     va_start(ap, format);
        //     Logv(info_log, format, ap);
        //     va_end(ap);
        // };
        
    } // namespace log
    
    namespace rocksdb
    {
        inline const char* RocksLogShorterFileName(const char* file) {
            // 18 is the length of "logging/logging.h".
            // If the name of this file changed, please change this number, too.
            return file + (sizeof(__FILE__) > 18 ? sizeof(__FILE__) - 18 : 0);
        }
        #define ROCKS_LOG_STRINGIFY(x) #x
        #define ROCKS_LOG_TOSTRING(x) ROCKS_LOG_STRINGIFY(x)
        #define ROCKS_LOG_PREPEND_FILE_LINE(FMT) \
            ("[%s:" ROCKS_LOG_TOSTRING(__LINE__) "] " FMT)

        // Don't inclide file/line info in HEADER level
        #define ROCKS_LOG_HEADER(LGR, FMT, ...) \
            latte::log::Log(latte::log::InfoLogLevel::HEADER_LEVEL, LGR, FMT, ##__VA_ARGS__)
        #define ROCKS_LOG_AT_LEVEL(LGR, LVL, FMT, ...)                           \
            latte::log::Log((LVL), (LGR), ROCKS_LOG_PREPEND_FILE_LINE(FMT), \
                                    RocksLogShorterFileName(__FILE__), ##__VA_ARGS__)
        #define ROCKS_LOG_DEBUG(LGR, FMT, ...) \
            ROCKS_LOG_AT_LEVEL((LGR), latte::log::InfoLogLevel::DEBUG_LEVEL, FMT, ##__VA_ARGS__)

        #define ROCKS_LOG_INFO(LGR, FMT, ...) \
            ROCKS_LOG_AT_LEVEL((LGR), latte::log::InfoLogLevel::INFO_LEVEL, FMT, ##__VA_ARGS__)

        #define ROCKS_LOG_WARN(LGR, FMT, ...) \
            ROCKS_LOG_AT_LEVEL((LGR), latte::log::InfoLogLevel::WARN_LEVEL, FMT, ##__VA_ARGS__)

        #define ROCKS_LOG_ERROR(LGR, FMT, ...) \
            ROCKS_LOG_AT_LEVEL((LGR), latte::log::InfoLogLevel::ERROR_LEVEL, FMT, ##__VA_ARGS__)

        #define ROCKS_LOG_FATAL(LGR, FMT, ...) \
            ROCKS_LOG_AT_LEVEL((LGR), latte::log::InfoLogLevel::FATAL_LEVEL, FMT, ##__VA_ARGS__)
    } // namespace rocksdb
    
    
    
} // namespace latte


#endif
