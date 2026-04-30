#pragma once
#include "LogEntry.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <deque>
#include <functional>
#include <iostream>

// ============================================================
// LEARN: Strategy Design Pattern
//
// Problem: We want MULTIPLE detection algorithms (frequency-based,
//          keyword-based, burst-based) but we don't want to rewrite
//          AnomalyDetector every time we add a new one.
//
// Solution: Define a common INTERFACE (abstract base class).
//           Each algorithm is a separate class that implements it.
//           AnomalyDetector holds a LIST of strategies and runs all of them.
//
// Benefit: Adding a new algorithm = add a new class. Nothing else changes.
//          This is the Open/Closed Principle: open for extension, closed for modification.
// ============================================================

struct Anomaly {
    std::string type;       // what kind of anomaly
    std::string detail;     // human-readable description
    int         severity;   // 1 (low) to 3 (critical)
};

// ============================================================
// LEARN: Abstract Base Class (Interface in C++)
// Pure virtual function (= 0) forces subclasses to implement it.
// Cannot instantiate IDetectionStrategy directly.
// ============================================================
class IDetectionStrategy {
public:
    virtual ~IDetectionStrategy() = default;

    // Every strategy must implement this
    virtual std::vector<Anomaly> analyze(const std::vector<LogEntry>& entries) = 0;

    // Every strategy has a name (for reporting)
    virtual std::string name() const = 0;
};


// ============================================================
// STRATEGY 1: Error Rate Detector
// ALGORITHM: Sliding Window
//
// LEARN: Sliding Window Technique
// Instead of recomputing from scratch for every window,
// we slide a "window" of fixed size over the data.
// Time Complexity: O(n) instead of O(n*k)
// ============================================================
class ErrorRateDetector : public IDetectionStrategy {
public:
    // windowSize = number of entries to look at at once
    // threshold  = how many errors in that window = anomaly
    ErrorRateDetector(int windowSize = 10, int threshold = 5)
        : windowSize_(windowSize), threshold_(threshold) {}

    std::string name() const override { return "ErrorRateDetector"; }

    std::vector<Anomaly> analyze(const std::vector<LogEntry>& entries) override {
        std::vector<Anomaly> anomalies;

        // ============================================================
        // LEARN: std::deque (Double-Ended Queue)
        // Efficient insert/remove at BOTH ends. Perfect for sliding windows.
        // deque.push_back()  → add to right
        // deque.pop_front()  → remove from left
        // ============================================================
        std::deque<LogLevel> window;
        int errorCount = 0;

        for (const auto& entry : entries) {
            // Add new entry to window
            window.push_back(entry.level);
            if (entry.level == LogLevel::ERROR || entry.level == LogLevel::CRITICAL)
                errorCount++;

            // Slide: remove oldest entry if window is full
            if ((int)window.size() > windowSize_) {
                if (window.front() == LogLevel::ERROR || window.front() == LogLevel::CRITICAL)
                    errorCount--;
                window.pop_front();
            }

            // Check threshold
            if (errorCount >= threshold_) {
                anomalies.push_back({
                    "HIGH_ERROR_RATE",
                    "Found " + std::to_string(errorCount) + " errors in last "
                        + std::to_string(windowSize_) + " entries",
                    2
                });
                // Reset to avoid duplicate consecutive alerts
                errorCount = 0;
                window.clear();
            }
        }
        return anomalies;
    }

private:
    int windowSize_;
    int threshold_;
};


// ============================================================
// STRATEGY 2: Keyword Detector
// ALGORITHM: Hash Map frequency counting
//
// LEARN: unordered_map (Hash Map)
// Stores key-value pairs. Lookup is O(1) average.
// Perfect for counting occurrences of strings.
// ============================================================
class KeywordDetector : public IDetectionStrategy {
public:
    KeywordDetector() {
        // These keywords in log messages indicate serious problems
        suspiciousKeywords_ = {
            "timeout", "failed", "crash", "null", "overflow",
            "denied", "corrupted", "leak", "segfault", "abort"
        };
    }

    std::string name() const override { return "KeywordDetector"; }

    std::vector<Anomaly> analyze(const std::vector<LogEntry>& entries) override {
        std::vector<Anomaly> anomalies;

        // ============================================================
        // LEARN: unordered_map<string, int>
        // key   = keyword string
        // value = how many times we've seen it
        // ============================================================
        std::unordered_map<std::string, int> keywordFreq;

        for (const auto& entry : entries) {
            std::string msgLower = toLower(entry.message);

            for (const auto& keyword : suspiciousKeywords_) {
                // Check if keyword appears in message
                if (msgLower.find(keyword) != std::string::npos) {
                    keywordFreq[keyword]++;
                }
            }
        }

        // Report any keyword that appears more than 3 times
        for (const auto& [keyword, count] : keywordFreq) {
            if (count > 3) {
                anomalies.push_back({
                    "REPEATED_KEYWORD",
                    "Keyword '" + keyword + "' appeared " + std::to_string(count) + " times",
                    count > 10 ? 3 : 2
                });
            }
        }

        return anomalies;
    }

private:
    std::vector<std::string> suspiciousKeywords_;

    std::string toLower(std::string s) {
        for (char& c : s) c = tolower(c);
        return s;
    }
};


// ============================================================
// STRATEGY 3: Source Burst Detector
// ALGORITHM: Frequency Map — detect if ONE source spams logs
// ============================================================
class SourceBurstDetector : public IDetectionStrategy {
public:
    std::string name() const override { return "SourceBurstDetector"; }

    std::vector<Anomaly> analyze(const std::vector<LogEntry>& entries) override {
        std::vector<Anomaly> anomalies;
        std::unordered_map<std::string, int> sourceCounts;

        for (const auto& entry : entries) {
            sourceCounts[entry.source]++;
        }

        int totalEntries = (int)entries.size();
        for (const auto& [source, count] : sourceCounts) {
            double percentage = (double)count / totalEntries * 100.0;
            // If a single source generated more than 60% of all logs → suspicious
            if (percentage > 60.0 && totalEntries > 5) {
                anomalies.push_back({
                    "SOURCE_LOG_BURST",
                    "Source '" + source + "' generated " + std::to_string(count)
                        + " entries (" + std::to_string((int)percentage) + "% of total)",
                    2
                });
            }
        }
        return anomalies;
    }
};


// ============================================================
// MAIN ANOMALY DETECTOR
// Holds all strategies and runs them.
// This is the "Context" in the Strategy Pattern.
// ============================================================
class AnomalyDetector {
public:
    AnomalyDetector() {
        // Register all strategies
        // LEARN: std::make_unique = smart pointer (auto memory management)
        // No manual new/delete needed — memory freed automatically!
        strategies_.push_back(std::make_unique<ErrorRateDetector>());
        strategies_.push_back(std::make_unique<KeywordDetector>());
        strategies_.push_back(std::make_unique<SourceBurstDetector>());
    }

    // Run ALL strategies and collect results
    std::vector<Anomaly> detect(const std::vector<LogEntry>& entries) {
        std::vector<Anomaly> allAnomalies;

        for (const auto& strategy : strategies_) {
            std::cout << "[AnomalyDetector] Running: " << strategy->name() << "\n";
            auto found = strategy->analyze(entries);
            // Append found anomalies to master list
            allAnomalies.insert(allAnomalies.end(), found.begin(), found.end());
        }

        return allAnomalies;
    }

private:
    // ============================================================
    // LEARN: vector of unique_ptr to base class
    // This is POLYMORPHISM: we store different strategy types
    // under one common pointer type (IDetectionStrategy*).
    // At runtime, the correct subclass method is called. (vtable)
    // ============================================================
    std::vector<std::unique_ptr<IDetectionStrategy>> strategies_;
};