


#include "logging.h"

namespace latte
{
    #define LATTE_OK 1
    int LATTE_ADD_STDOUT_LOG(std::string tag, LogLevel level) {
        LogLevelLoggerManager& instance = LogLevelLoggerManager::getInstance();
        instance.addLogger(tag, std::make_shared<ConsoleLogger>(level));
        return LATTE_OK;
    }

    int LATTE_ADD_FILE_LOG(std::string tag, std::string file_name, LogLevel level) {
        LogLevelLoggerManager& instance = LogLevelLoggerManager::getInstance();
        instance.addLogger(tag, std::make_shared<FileLogger>(file_name, level));
        return LATTE_OK;
    }

    // template<typename T, typename... Args>
    // void LATTE_LOG(const std::string& tag, LogLevel level, const std::string& format, T value,  Args&&... args) {
    //     LogLevelLoggerManager& instance = LogLevelLoggerManager::getInstance();
    //     instance.log(tag, level, format, value, std::forward<Args>(args)...);
    // }

    

    // static int LATTE_ADD_FILE_LOG(std::string tag, Logger* logger);
} // namespace latte
