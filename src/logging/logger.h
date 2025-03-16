
#pragma once
#include <cstdarg>
#include <string>

namespace latte
{
    class Logger {
        public:
            Logger() = default;
            Logger(const Logger&) = delete;
            Logger& operator=(const Logger&) = delete;

            virtual ~Logger() {};
        
            virtual void logv(const std::string& message) = 0;

    };
    

} // namespace latte
