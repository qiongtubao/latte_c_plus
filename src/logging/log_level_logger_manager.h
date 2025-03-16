

#include "log_level.h"
#include <map>
#include "autovector/autovector.h"
#include <memory>
#include <mutex>
#include <vector>

namespace latte
{
    class LogLevelLoggerManager {
        private:
            std::map<std::string, std::vector<std::shared_ptr<LogLevelLogger>>> loggers;
            std::mutex mtx;

            LogLevelLoggerManager() = default;
            LogLevelLoggerManager(const LogLevelLoggerManager&) = delete;
            LogLevelLoggerManager& operator=(const LogLevelLoggerManager&) = delete;
        public:
            static LogLevelLoggerManager& getInstance() {
                static LogLevelLoggerManager instance;
                return instance;
            }

            void addLogger(const std::string& tag, std::shared_ptr<LogLevelLogger> logger) {
                std::lock_guard<std::mutex> lock(mtx);
                loggers[tag].push_back(logger);
            }
            template<typename T, typename... Args>
            void log(const std::string& tag, LogLevel level, const std::string& format, T value, Args... args) {
                std::lock_guard<std::mutex> lock(mtx);
                auto it = loggers.find(tag);
                if (it != loggers.end()) {
                    for(auto& logger: it->second)
                    {
                        logger->log(level, format, value, args...);
                    }                
                } 
            }

            // void log(const std::string& tag, LogLevel level, const std::string& message) {
            //     std::lock_guard<std::mutex> lock(mtx);
            //     auto it = loggers.find(tag);
            //     if (it != loggers.end()) {
            //         for(auto& logger: it->second)
            //         {
            //             logger->log(level, message);
            //         }                
            //     } 
            // }


    };
} // namespace latte

