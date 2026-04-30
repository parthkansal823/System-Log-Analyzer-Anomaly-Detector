#pragma once // It's an preprocessor directive which ensure the header file is included only once.
#include <string>

// using namespace std; --> Don't use

// enum (enumeration) is a user-defined data type that lets you give names to a set of integer constants.
enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    CRITICAL,
    UNKNOWN
};

// struct vs class --> struct for plain text and class for behavior
// using struct here
struct LogEntry{
    std::string raw;
    std::string timestamp;
    LogLevel level;
    std::string source;
    std::string message;


    std::string levelToString() const {
        switch (level) {
            case LogLevel::INFO:     return "INFO";
            case LogLevel::WARNING:  return "WARNING";
            case LogLevel::ERROR:    return "ERROR";
            case LogLevel::CRITICAL: return "CRITICAL";
            default:                 return "UNKNOWN";
        }
    }
};