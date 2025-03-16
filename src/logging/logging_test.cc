
#include <gtest/gtest.h>
#include "logger.h"
#include "logging.h"
namespace latte
{
    TEST(LOGGING_TEST, Empty){ 
        // LogLevelLoggerManager& instance = LogLevelLoggerManager::getInstance();
        // instance.addLogger("latte", std::make_shared<ConsoleLogger>(LogLevel::DEBUG));
        // instance.addLogger("latte", std::make_shared<FileLogger>("latte_test.log",LogLevel::DEBUG));
        
        LATTE_LOG("latte", LogLevel::INFO, "这是一个数字{}", 123);
        LATTE_ADD_STDOUT_LOG("latte", LogLevel::DEBUG);
        LATTE_ADD_FILE_LOG("latte", "latte_test.log", LogLevel::DEBUG);
        LATTE_LOG("latte", LogLevel::INFO, "这是一个数字{}", 123);
    }
} // namespace latte
