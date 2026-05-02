// ============================================================
// System Log Analyzer & Anomaly Detector
// Author: [Your Name]
// Built for: Philips Software Intern Application
//
// LEARN: This is the entry point of every C++ program.
// Execution always starts at main().
// argc = argument count, argv = argument values (as C-strings)
// ============================================================

#include <iostream>
#include <string>
#include <filesystem>   // C++17: path, directory management

#include "LogParser.h"
#include "AnomalyDetector.h"
#include "ReportGenerator.h"
#include "FileWatcher.h"

// ============================================================
// LEARN: Namespaces
// 'namespace fs' is a short alias for std::filesystem.
// Avoids typing std::filesystem:: everywhere.
// ============================================================
namespace fs = std::filesystem;

// Forward declarations
void runAnalysis(const std::string& logFile, const std::string& reportDir);
void printUsage(const std::string& programName);

int main(int argc, char* argv[]) {
    std::cout << "============================================\n";
    std::cout << "   System Log Analyzer v1.0                \n";
    std::cout << "============================================\n\n";

    // ============================================================
    // LEARN: Command-line arguments
    // argv[0] = program name
    // argv[1] = first user-supplied argument
    // We support two modes:
    //   Mode 1: Analyze once  →  ./LogAnalyzer <logfile>
    //   Mode 2: Watch mode    →  ./LogAnalyzer <logfile> --watch
    // ============================================================

    // Default: use sample log if no argument given
    std::string logFile   = "logs/sample.log";
    std::string reportDir = "reports/";
    bool        watchMode = false;

    if (argc >= 2) logFile   = argv[1];
    if (argc >= 3 && std::string(argv[2]) == "--watch") watchMode = true;

    // Create reports directory if it doesn't exist (C++17)
    fs::create_directories(reportDir);

    if (watchMode) {
        // ============================================================
        // LEARN: FileWatcher + lambda callback
        // The lambda is called every time the file changes.
        // This is the Observer Pattern: FileWatcher notifies us of events.
        // ============================================================
        std::cout << "[Main] Starting WATCH MODE. Press Ctrl+C to stop.\n\n";

        FileWatcher watcher(logFile, 3);
        watcher.start([&](const std::string& changedFile) {
            std::cout << "\n[Main] Re-analyzing due to file change...\n";
            runAnalysis(changedFile, reportDir);
        });

        // Run initial analysis
        runAnalysis(logFile, reportDir);

        // Keep main thread alive
        std::cout << "Watching for changes... (Ctrl+C to exit)\n";
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } else {
        // One-shot analysis mode
        runAnalysis(logFile, reportDir);
    }

    return 0;
}

// ============================================================
// Core analysis pipeline:
// File → Parse → Detect Anomalies → Generate Report
// ============================================================
void runAnalysis(const std::string& logFile, const std::string& reportDir) {
    std::cout << "[Main] Analyzing: " << logFile << "\n\n";

    // STEP 1: Parse the log file
    auto entries = LogParser::parseFile(logFile);
    if (entries.empty()) {
        std::cerr << "[Main] No entries to analyze.\n";
        return;
    }

    // STEP 2: Detect anomalies
    AnomalyDetector detector;
    auto anomalies = detector.detect(entries);

    // STEP 3: Print summary to console
    ReportGenerator::printConsoleSummary(entries, anomalies);

    // STEP 4: Write full report to file
    std::string reportPath = reportDir + "report.txt";
    ReportGenerator::generateReport(reportPath, entries, anomalies);
}

void printUsage(const std::string& prog) {
    std::cout << "Usage:\n";
    std::cout << "  " << prog << " <logfile>           (analyze once)\n";
    std::cout << "  " << prog << " <logfile> --watch   (watch for changes)\n";
}