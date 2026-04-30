# System Log Analyzer & Anomaly Detector

A real-time C++ log monitoring tool that parses system log files, detects anomalies using multiple algorithms, and generates structured reports. Built with C++17, OOP design patterns, and OS-level file watching.

---

## Table of Contents

- [Features](#features)
- [Project Structure](#project-structure)
- [Architecture](#architecture)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Build on Linux / macOS](#build-on-linux--macos)
  - [Build on Windows](#build-on-windows)
- [Usage](#usage)
- [Log Format](#log-format)
- [Detection Algorithms](#detection-algorithms)
- [Sample Output](#sample-output)
- [Extending the Project](#extending-the-project)
- [Concepts Covered](#concepts-covered)
- [Tech Stack](#tech-stack)

---

## Features

- **Real-time log parsing** — reads and parses structured log files line by line
- **3 anomaly detection algorithms** — sliding window, keyword frequency, and source burst detection
- **Watch mode** — monitors log file for changes using OS-level `stat()` syscall and re-analyzes automatically
- **Structured report generation** — saves full analysis with ASCII charts to `reports/report.txt`
- **Modular & extensible** — add new detection algorithms without touching existing code (Strategy pattern)

---

## Project Structure

```
LogAnalyzer/
├── src/
│   ├── main.cpp              # Entry point — CLI args, pipeline orchestration
│   ├── LogEntry.h            # Data model: struct + LogLevel enum
│   ├── LogParser.h           # File I/O + string parsing → vector<LogEntry>
│   ├── AnomalyDetector.h     # Strategy pattern + 3 detection algorithms
│   ├── ReportGenerator.h     # File I/O report writer + console summary
│   └── FileWatcher.h         # OS-level file monitoring with std::thread
├── logs/
│   └── sample.log            # Sample log file for testing
├── reports/
│   └── report.txt            # Auto-generated after each analysis run
├── CMakeLists.txt            # CMake build configuration (C++17)
└── README.md
```

---

## Architecture

```
┌─────────────┐     ┌────────────┐      ┌──────────────────┐
│  Log File   │────▶│ LogParser  │────▶│ vector<LogEntry> │
└─────────────┘     └────────────┘      └────────┬─────────┘
                                                 │
                                                 ▼
                                   ┌───────────────────────────┐
                                   │      AnomalyDetector      │
                                   │  ┌──────────────────────┐ │
                                   │  │  ErrorRateDetector   │ │  ← Sliding Window
                                   │  │  KeywordDetector     │ │  ← Hash Map
                                   │  │  SourceBurstDetector │ │  ← Frequency
                                   │  └──────────────────────┘ │
                                   └────────────┬──────────────┘
                                                │
                                                ▼
                                   ┌───────────────────────────┐
                                   │     ReportGenerator       │
                                   │  Console Summary          │
                                   │  reports/report.txt       │
                                   └───────────────────────────┘

Watch Mode:
┌─────────────┐  file changed    ┌──────────────────────────┐
│ FileWatcher │────────────────▶ │  Re-run entire pipeline  │
│ (thread)    │  stat() polling  │                          │
└─────────────┘                  └──────────────────────────┘
```

---

## Getting Started

### Prerequisites

| Tool | Version | Purpose |
|------|---------|---------|
| g++ or clang++ | C++17+ | Compiler |
| CMake | 3.14+ | Build system (optional) |
| make | any | Build tool (Linux/macOS) |

---

### Build on Linux / macOS

**Option A — g++ directly (quickest)**

```bash
# Clone the repo
git clone https://github.com/parthkansal823/System-Log-Analyzer-Anomaly-Detector.git
cd System-Log-Analyzer-Anomaly-Detector

# Compile
g++ -std=c++17 -Wall -Wextra -Isrc src/main.cpp -o LogAnalyzer

# Run
./LogAnalyzer logs/sample.log
```

**Option B — CMake (recommended)**

```bash
mkdir build && cd build
cmake ..
make
./LogAnalyzer ../logs/sample.log
```

---

### Build on Windows

**With MSVC (Visual Studio)**

```cmd
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
Release\LogAnalyzer.exe ..\logs\sample.log
```

**With MinGW / g++**

```cmd
g++ -std=c++17 -Wall -Isrc src/main.cpp -o LogAnalyzer.exe
LogAnalyzer.exe logs\sample.log
```

> **Note:** `FileWatcher.h` uses POSIX `stat()`. On Windows, replace with `_stat()` from `<sys/stat.h>` — the API is nearly identical.

---

## Usage

```
./LogAnalyzer <logfile>             Analyze log file once
./LogAnalyzer <logfile> --watch     Watch for changes and re-analyze automatically
./LogAnalyzer                       Uses logs/sample.log by default
```

**Examples:**

```bash
# Analyze once
./LogAnalyzer logs/sample.log

# Watch mode — re-runs every time the file changes
./LogAnalyzer logs/sample.log --watch

# Analyze a custom log file
./LogAnalyzer /var/log/syslog
```

---

## Log Format

The parser expects logs in this format:

```
[TIMESTAMP] [LEVEL] [SOURCE] MESSAGE
```

| Field | Description | Example |
|-------|-------------|---------|
| TIMESTAMP | Date and time | `2024-01-15 10:23:45` |
| LEVEL | Severity | `INFO`, `WARNING`, `ERROR`, `CRITICAL` |
| SOURCE | Module/component name | `NetworkModule` |
| MESSAGE | Log message text | `Connection timeout after 5000ms` |

**Example log lines:**

```
[2024-01-15 09:00:01] [INFO] [StartupManager] System boot sequence initiated
[2024-01-15 09:01:18] [ERROR] [NetworkModule] Connection timeout after 5000ms
[2024-01-15 09:02:15] [CRITICAL] [NetworkModule] Service crash — null pointer dereference
```

---

## Detection Algorithms

### 1. Error Rate Detector — Sliding Window

Scans a moving window of N log entries. If the number of `ERROR` or `CRITICAL` entries exceeds a threshold, an anomaly is raised.

```
Window size : 10 entries
Threshold   : 5 errors → anomaly triggered

[INFO][ERROR][ERROR][WARNING][ERROR][ERROR][CRITICAL] → ⚠ HIGH_ERROR_RATE
```

**Time complexity:** O(n) — each entry is added and removed from the deque exactly once.

---

### 2. Keyword Detector — Hash Map Frequency

Scans all log messages for suspicious keywords. Uses `std::unordered_map` (O(1) lookup) to count occurrences. Raises an anomaly if any keyword appears more than 3 times.

**Monitored keywords:**
`timeout`, `failed`, `crash`, `null`, `overflow`, `denied`, `corrupted`, `leak`, `segfault`, `abort`

---

### 3. Source Burst Detector — Frequency Analysis

Detects if a single source module is generating a disproportionate share of all log entries (> 60% of total). Indicates a misbehaving or looping component.

---

## Sample Output

**Console:**

```
============================================
   System Log Analyzer v1.0
============================================

[LogParser] Parsed 30 log entries.
[AnomalyDetector] Running: ErrorRateDetector
[AnomalyDetector] Running: KeywordDetector
[AnomalyDetector] Running: SourceBurstDetector

╔══════════════════════════════════════╗
║       LOG ANALYSIS SUMMARY           ║
╚══════════════════════════════════════╝
  Total entries analyzed : 30
  Anomalies detected     : 3
  INFO                   : 14
  WARNING                : 3
  ERROR                  : 9
  CRITICAL               : 4

  ⚠ ANOMALIES FOUND:
    [HIGH]     HIGH_ERROR_RATE  : Found 5 errors in last 10 entries
    [HIGH]     REPEATED_KEYWORD : Keyword 'timeout' appeared 4 times
    [HIGH]     SOURCE_LOG_BURST : Source 'NetworkModule' generated 18 entries (60% of total)

[ReportGenerator] Report saved to: reports/report.txt
```

**Generated report (`reports/report.txt`):**

```
============================================================
         SYSTEM LOG ANALYZER - ANALYSIS REPORT
============================================================
Generated: 2026-04-15 14:32:01

--- SUMMARY ---
Total log entries : 30
Anomalies found   : 3

--- ANOMALIES ---
  [    HIGH] HIGH_ERROR_RATE           → Found 5 errors in last 10 entries
  [    HIGH] REPEATED_KEYWORD          → Keyword 'timeout' appeared 4 times
  [    HIGH] SOURCE_LOG_BURST          → Source 'NetworkModule' generated 18 entries

--- LOG LEVEL DISTRIBUTION ---
INFO       | ############## 14
WARNING    | ### 3
ERROR      | ######### 9
CRITICAL   | #### 4

--- TOP LOG SOURCES ---
  1. NetworkModule (18 entries)
  2. HealthMonitor (4 entries)
  3. StartupManager (3 entries)
  4. DatabaseDriver (3 entries)
  5. MemoryManager (2 entries)
```

---

## Extending the Project

Adding a new detection algorithm takes 3 steps — no existing code needs to change.

**Step 1 — Create a new strategy class in `AnomalyDetector.h`:**

```cpp
class MyCustomDetector : public IDetectionStrategy {
public:
    std::string name() const override { return "MyCustomDetector"; }

    std::vector<Anomaly> analyze(const std::vector<LogEntry>& entries) override {
        std::vector<Anomaly> anomalies;
        // your detection logic here
        return anomalies;
    }
};
```

**Step 2 — Register it in `AnomalyDetector`'s constructor:**

```cpp
AnomalyDetector() {
    strategies_.push_back(std::make_unique<ErrorRateDetector>());
    strategies_.push_back(std::make_unique<KeywordDetector>());
    strategies_.push_back(std::make_unique<SourceBurstDetector>());
    strategies_.push_back(std::make_unique<MyCustomDetector>()); // ← add here
}
```

**Step 3 — Rebuild and run.** Done.

---

## Concepts Covered

| Concept | Where Used |
|---------|-----------|
| OOP — Abstract base class | `IDetectionStrategy` in `AnomalyDetector.h` |
| Strategy design pattern | `AnomalyDetector` + 3 concrete strategies |
| Observer pattern | `FileWatcher` callback mechanism |
| Sliding window algorithm | `ErrorRateDetector` |
| Hash map frequency counting | `KeywordDetector`, `SourceBurstDetector` |
| Smart pointers (`unique_ptr`) | Strategy vector in `AnomalyDetector` |
| File I/O (`ifstream`/`ofstream`) | `LogParser`, `ReportGenerator` |
| OS syscall (`stat()`) | `FileWatcher` |
| Multithreading (`std::thread`) | `FileWatcher` background watch loop |
| Lambda functions | Sorting, FileWatcher callback |
| C++17 `std::filesystem` | Directory creation in `main.cpp` |

---

## Tech Stack

![C++](https://img.shields.io/badge/C++-17-00599C?style=flat&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.14+-064F8C?style=flat&logo=cmake&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey?style=flat)

- **Language:** C++17
- **STL:** `vector`, `deque`, `unordered_map`, `thread`, `filesystem`, `functional`
- **Design Patterns:** Strategy, Observer
- **OS API:** POSIX `stat()` for file metadata
- **Build:** CMake 3.14+ / g++ direct
- **Platform:** Linux, macOS, Windows (MinGW or MSVC)

---

## Author

**Parth Kansal**
- GitHub: [@parthkansal823](https://github.com/parthkansal823)
- LinkedIn: [kparth23](https://www.linkedin.com/in/kparth23/)
