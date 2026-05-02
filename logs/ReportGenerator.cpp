#pragma once
#include "LogEntry.h"
#include "AnomalyDetector.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <ctime>

// ============================================================
// LEARN: Report Generation with File I/O
// ofstream = output file stream (write to file)
// We write a structured text report summarizing all findings.
// ============================================================

class ReportGenerator {
public:

    // Generates a full analysis report and saves to disk
    static void generateReport(
        const std::string&          outputPath,
        const std::vector<LogEntry>& entries,
        const std::vector<Anomaly>&  anomalies)
    {
        // ============================================================
        // LEARN: ofstream
        // std::ios::out  → open for writing
        // std::ios::trunc → overwrite if file exists
        // ============================================================
        std::ofstream report(outputPath, std::ios::out | std::ios::trunc);

        if (!report.is_open()) {
            std::cerr << "[ReportGenerator] ERROR: Cannot write to: " << outputPath << "\n";
            return;
        }

        writeHeader(report);
        writeSummary(report, entries, anomalies);
        writeAnomalies(report, anomalies);
        writeStatistics(report, entries);
        writeTopSources(report, entries);
        writeFooter(report);

        report.close();
        std::cout << "[ReportGenerator] Report saved to: " << outputPath << "\n";
    }

    // Also print a short summary to console
    static void printConsoleSummary(
        const std::vector<LogEntry>& entries,
        const std::vector<Anomaly>&  anomalies)
    {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════╗\n";
        std::cout << "║       LOG ANALYSIS SUMMARY           ║\n";
        std::cout << "╚══════════════════════════════════════╝\n";
        std::cout << "  Total entries analyzed : " << entries.size()  << "\n";
        std::cout << "  Anomalies detected     : " << anomalies.size() << "\n";

        // Count by level
        auto counts = countByLevel(entries);
        std::cout << "  INFO                  : " << counts["INFO"]     << "\n";
        std::cout << "  WARNING               : " << counts["WARNING"]  << "\n";
        std::cout << "  ERROR                 : " << counts["ERROR"]    << "\n";
        std::cout << "  CRITICAL              : " << counts["CRITICAL"] << "\n";

        if (!anomalies.empty()) {
            std::cout << "\n  ⚠ ANOMALIES FOUND:\n";
            for (const auto& a : anomalies) {
                std::string severity = a.severity == 3 ? "CRITICAL" :
                                       a.severity == 2 ? "HIGH"     : "LOW";
                std::cout << "    [" << severity << "] " << a.type << ": " << a.detail << "\n";
            }
        } else {
            std::cout << "\n  ✓ No anomalies detected.\n";
        }
        std::cout << "\n";
    }

private:

    static void writeHeader(std::ofstream& out) {
        out << "============================================================\n";
        out << "         SYSTEM LOG ANALYZER - ANALYSIS REPORT             \n";
        out << "============================================================\n";
        out << "Generated: " << getCurrentTime() << "\n\n";
    }

    static void writeSummary(std::ofstream& out,
                              const std::vector<LogEntry>& entries,
                              const std::vector<Anomaly>&  anomalies)
    {
        out << "--- SUMMARY ---\n";
        out << "Total log entries : " << entries.size()   << "\n";
        out << "Anomalies found   : " << anomalies.size() << "\n\n";
    }

    static void writeAnomalies(std::ofstream& out, const std::vector<Anomaly>& anomalies) {
        out << "--- ANOMALIES ---\n";
        if (anomalies.empty()) {
            out << "No anomalies detected.\n\n";
            return;
        }

        // ============================================================
        // LEARN: Sorting with a lambda comparator
        // std::sort takes a custom comparator function.
        // Here we sort anomalies by severity (highest first).
        // Lambda: [capture](params) { body }
        // ============================================================
        auto sorted = anomalies; // copy so we don't modify original
        std::sort(sorted.begin(), sorted.end(),
            [](const Anomaly& a, const Anomaly& b) {
                return a.severity > b.severity; // descending
            });

        for (const auto& a : sorted) {
            std::string sev = a.severity == 3 ? "CRITICAL" :
                              a.severity == 2 ? "HIGH"     : "LOW";
            out << "  [" << std::setw(8) << sev << "] "
                << std::setw(25) << std::left << a.type
                << " → " << a.detail << "\n";
        }
        out << "\n";
    }

    static void writeStatistics(std::ofstream& out, const std::vector<LogEntry>& entries) {
        out << "--- LOG LEVEL DISTRIBUTION ---\n";
        auto counts = countByLevel(entries);
        for (const auto& [level, count] : counts) {
            // Draw a simple ASCII bar chart
            int barLen = std::min(count, 40);
            out << std::setw(10) << std::left << level << " | ";
            out << std::string(barLen, '#') << " " << count << "\n";
        }
        out << "\n";
    }

    static void writeTopSources(std::ofstream& out, const std::vector<LogEntry>& entries) {
        out << "--- TOP LOG SOURCES ---\n";

        std::unordered_map<std::string, int> sourceCounts;
        for (const auto& e : entries) sourceCounts[e.source]++;

        // ============================================================
        // LEARN: Sorting a map by value
        // Maps don't sort by value natively.
        // Step 1: Copy into a vector of pairs
        // Step 2: Sort the vector
        // ============================================================
        std::vector<std::pair<std::string, int>> sorted(sourceCounts.begin(), sourceCounts.end());
        std::sort(sorted.begin(), sorted.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        int rank = 1;
        for (const auto& [src, cnt] : sorted) {
            out << "  " << rank++ << ". " << src << " (" << cnt << " entries)\n";
            if (rank > 5) break; // top 5 only
        }
        out << "\n";
    }

    static void writeFooter(std::ofstream& out) {
        out << "============================================================\n";
        out << "                    END OF REPORT                          \n";
        out << "============================================================\n";
    }

    // Helper: count entries by log level
    static std::unordered_map<std::string, int> countByLevel(const std::vector<LogEntry>& entries) {
        std::unordered_map<std::string, int> counts = {
            {"INFO", 0}, {"WARNING", 0}, {"ERROR", 0}, {"CRITICAL", 0}
        };
        for (const auto& e : entries) counts[e.levelToString()]++;
        return counts;
    }

    // Helper: get current timestamp as string
    static std::string getCurrentTime() {
        time_t now = time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return std::string(buf);
    }
};