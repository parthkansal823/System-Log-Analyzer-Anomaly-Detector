#pragma once
#include <string>
#include <functional>
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/stat.h>    // POSIX file info (works on Linux/macOS)

// ============================================================
// LEARN: OS-Level File Watching
//
// The OS keeps metadata about every file (size, modified time).
// We use stat() syscall to read this metadata.
// By polling every N seconds and comparing the "last modified" time,
// we detect when a file changes — like how log-monitoring tools work!
//
// On Windows you'd use: FindFirstChangeNotification() or ReadDirectoryChangesW()
// On Linux/macOS: stat() polling or inotify (more efficient)
// ============================================================

class FileWatcher {
public:
    // ============================================================
    // LEARN: std::function — storing any callable (function, lambda)
    // This lets callers pass a callback: "call THIS when file changes"
    // This is the Observer Pattern in action!
    // ============================================================
    using ChangeCallback = std::function<void(const std::string& filePath)>;

    FileWatcher(const std::string& filePath, int pollIntervalSeconds = 2)
        : filePath_(filePath),
          pollInterval_(pollIntervalSeconds),
          running_(false),
          lastModTime_(0)
    {}

    // Destructor ensures watcher thread stops cleanly
    ~FileWatcher() { stop(); }

    // Start watching in a BACKGROUND THREAD
    void start(ChangeCallback onChanged) {
        running_ = true;
        lastModTime_ = getLastModifiedTime();

        // ============================================================
        // LEARN: std::thread
        // Spawns a new thread running the given function.
        // Lambda captures 'this' and 'onChanged' by value/reference.
        // The thread runs watchLoop() concurrently with main program.
        // ============================================================
        watchThread_ = std::thread([this, onChanged]() {
            watchLoop(onChanged);
        });

        std::cout << "[FileWatcher] Watching: " << filePath_
                  << " (every " << pollInterval_ << "s)\n";
    }

    void stop() {
        running_ = false;
        if (watchThread_.joinable())
            watchThread_.join(); // wait for thread to finish
    }

private:
    std::string filePath_;
    int         pollInterval_;
    bool        running_;
    time_t      lastModTime_;
    std::thread watchThread_;

    void watchLoop(ChangeCallback onChanged) {
        while (running_) {
            // ============================================================
            // LEARN: std::this_thread::sleep_for
            // Pause execution without busy-waiting (CPU-friendly)
            // std::chrono::seconds(N) = N second duration
            // ============================================================
            std::this_thread::sleep_for(std::chrono::seconds(pollInterval_));

            time_t currentModTime = getLastModifiedTime();

            if (currentModTime != lastModTime_ && currentModTime != 0) {
                lastModTime_ = currentModTime;
                std::cout << "[FileWatcher] Change detected in: " << filePath_ << "\n";
                onChanged(filePath_); // trigger the callback!
            }
        }
    }

    // Uses OS syscall to get file's last-modified timestamp
    time_t getLastModifiedTime() {
        struct stat fileStat;
        if (stat(filePath_.c_str(), &fileStat) == 0) {
            return fileStat.st_mtime; // modification time in seconds since epoch
        }
        return 0; // file not accessible
    }
};