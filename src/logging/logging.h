#pragma once
#include "log_level.h"
#include <map>
#include <string>
#include "autovector/autovector.h"
#include "log_level_logger_manager.h"
#include <memory>

#if defined(__GNUC__) || defined(__clang__)
#define LATTE_PRINTF_FORMAT_ATTR(format_param, dots_param) \
  __attribute__((__format__(__printf__, format_param, dots_param)))
#else
#define LATTE_PRINTF_FORMAT_ATTR(format_param, dots_param)
#endif

namespace latte // Logger 相关的方法
{
    // static std::map<std::string, std::shared_ptr<autovector<LogLevelLogger*>>> global_logger_factory = {};

    int LATTE_ADD_STDOUT_LOG(std::string, LogLevel level);

    int LATTE_ADD_FILE_LOG(std::string tag, std::string file_name, LogLevel level);

    #define LATTE_LOG(tag, level, format, ...) \
    do { \
        LogLevelLoggerManager::getInstance().log(tag, level, format, ##__VA_ARGS__); \
    } while(0);
    // void FileLogFlush(FileLogger* info_log);

} // namespace latte Logger