#pragma once
#include "logger.h"
#include <mutex>
#include <iostream>
#include <fstream>
#include <sstream>
namespace latte
{
    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };
    class LogLevelLogger: public Logger {
        protected:
            LogLevel min_level;
            std::mutex mtx;

            static std::string LogLevelToString(LogLevel level) {
                switch(level) {
                    case LogLevel::DEBUG: return "DEBUG";
                    case LogLevel::INFO: return "INFO";
                    case LogLevel::WARN: return "WARN";
                    case LogLevel::ERROR: return "ERROR";
                    case LogLevel::FATAL: return "FATAL";
                    default: return "UNKNOWN";
                }
            }
        public:
            LogLevelLogger(LogLevel minLevel = LogLevel::DEBUG): min_level(minLevel) {}
            void log(LogLevel level, const std::string& message) {
                if (level < getMinLevel()) return;
                std::ostringstream stream;
                formatLogMessage(stream, "[{}] {}", LogLevelToString(level), message);
                logv(stream.str());
            }
            template<typename... Args>
            void log(LogLevel level, const std::string& format, Args... args) {
                if (level < getMinLevel()) return;

                std::ostringstream stream;
                formatLogMessage(stream, format, args...);
                log(level, stream.str());
            }
            LogLevel getMinLevel() const { return min_level; }
            
        private:
            template<typename T, typename... Args>
            void formatLogMessage(std::ostringstream& stream, const std::string& format, T value, Args... args) {
                size_t pos = format.find("{}");
                if(pos != std::string::npos) {
                    stream << format.substr(0, pos) << value;
                    formatLogMessage(stream, format.substr(pos + 2), args...);
                }
            }

            void formatLogMessage(std::ostringstream& stream, const std::string& format) {
                stream << format;
            }
    };
    class ConsoleLogger : public LogLevelLogger {
        public:
            ConsoleLogger(LogLevel min_level): LogLevelLogger(min_level) {
               
            };

            void logv(const std::string& message) override {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << message << std::endl; 
            }

           
    };
    class FileLogger : public LogLevelLogger {
        public:
            FileLogger( std::string filename, LogLevel level_): LogLevelLogger(level_), fileStream(filename, std::ios::out | std::ios::app) {
                
            }
            void logv(const std::string& message) override {
                std::lock_guard<std::mutex> lock(mtx);
                if (fileStream.is_open()) {
                    fileStream  << message << std::endl;
                }
            }
            ~FileLogger() {
                if (fileStream.is_open()) {
                    fileStream.close();
                }
            }
        private:
            std::ofstream fileStream;
    };
    
} // namespace latte
