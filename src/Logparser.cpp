// THis will convert raw text to LogEntry objects
#pragma once
#include "LogEntry.cpp"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

class LogParser {
public:
    //static means no object is created and is called directly
    // const means value cant be changed
    static std::vector<LogEntry> parseFile(const std::string& filePath) {
        std::vector<LogEntry> entries;

        std::ifstream file(filePath);  // input -- read file
        if (!file.is_open()) {
            std::cerr << "[LogParser] ERROR: Cannot open file: " << filePath << "\n";
            return entries;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                entries.push_back(parseLine(line));
            }
        }

        file.close(); // always close the file!
        std::cout << "[LogParser] Parsed " << entries.size() << " log entries.\n";
        return entries;
    }


    static LogEntry parseLine(const std::string& line) {
        LogEntry entry;
        entry.raw = line;
        // size_t == special datatype which used for count, unsigned integer values only
        size_t pos = 0;
        // npos is a special constant used with C++ strings
        // It represents “not found” or “no position”

        // Extract TIMESTAMP 
        size_t start = line.find('[', pos);
        size_t end   = line.find(']', start);
        if (start != std::string::npos && end != std::string::npos) {
            entry.timestamp = line.substr(start + 1, end - start - 1);
            pos = end + 1;
        }

        // Extract LEVEL
        start = line.find('[', pos);
        end   = line.find(']', start);
        if (start != std::string::npos && end != std::string::npos) {
            std::string levelStr = line.substr(start + 1, end - start - 1);
            entry.level = parseLevel(levelStr);
            pos = end + 1;
        }

        // Extract SOURCE
        start = line.find('[', pos);
        end   = line.find(']', start);
        if (start != std::string::npos && end != std::string::npos) {
            entry.source = line.substr(start + 1, end - start - 1);
            pos = end + 1;
        }

        // Remaining text is the MESSAGE
        if (pos < line.size()) {
            entry.message = line.substr(pos);
            // Trim leading space
            if (!entry.message.empty() && entry.message[0] == ' ')
                entry.message = entry.message.substr(1);
        }

        return entry;
    }

private:
    static LogLevel parseLevel(const std::string& lvl) {
        if (lvl == "INFO")     return LogLevel::INFO;
        if (lvl == "WARNING")  return LogLevel::WARNING;
        if (lvl == "ERROR")    return LogLevel::ERROR;
        if (lvl == "CRITICAL") return LogLevel::CRITICAL;
        return LogLevel::UNKNOWN;
    }
};