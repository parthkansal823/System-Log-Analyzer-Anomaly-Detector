#pragma once
#include "LogEntry.cpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>

struct Anomaly {
    std::string type;
    std::string detail;
    int         severity;
};

/*
Abstract Base Class -->
class that cannot be intialized
Just an blueprint/interface
Should have atleast one pure virtual function

virtual is keyword which enables runtime polymorphism.
1. Runtime polymorphism means: the function call is decided at runtime (while the program is running), not before.
same function, different output (runtime pe decide hota hai)
   happens in two ways:
    1. Function Overriding == Base class has a virtual function. Derived class redefines (overrides) it.
    2. Pure virtual function = function jiska base class me koi implementation nahi hota, aur child class ko zaroor implement karna padta hai
        virtual void functionName() = 0;


2. Compile-Time Polymorphism
    1. Function overloading
    2. Operator overloading
*/

class DetectionStrategy{
public:
    virtual ~DetectionStrategy() = default;
    // Above line is an destructor with virtual meaning each its compulsory for derived classes also.
    // This let fir derived destructor runs, Then base destructor runs


    // Every strategy has a name (for reporting)
    virtual std::string name() const = 0;

    // Every strategy must implement this
    virtual std::vector<Anomaly> analyse(const std::vector<LogEntry>& entries) = 0;
};


class ErrorRate: public DetectionStrategy{
private:
    int windowSize_;
    int threshold_;

public:
    // constructor
    // Agar user value na de → ye values use hongi ErrorRate e1; then 10 and 5
    // ErrorRate e2(20, 8); then 20 and 8
    ErrorRate(int windowSize = 10, int threshold = 5): windowSize_(windowSize), threshold_(threshold){}

    std::string name() const override{
        return "ErrorRateDetector";
    }

    std::vector<Anomaly> analyse(const std::vector<LogEntry>& entries) override {
        std::vector<Anomaly> anamalies;

        std::deque<LogLevel> window;
        int errorCount = 0;

        for(const auto& entry: entries){
            window.push_back(entry.level);
            if(entry.level == LogLevel::ERROR || entry.level == LogLevel::CRITICAL){
                errorCount++;
            }

            if((int)window.size()>windowSize_){
                if (window.front() == LogLevel::ERROR || window.front() == LogLevel::CRITICAL)
                    errorCount--;
                window.pop_front();                
            }

            if (errorCount >= threshold_) {
                anamalies.push_back({"HIGH_ERROR_RATE",
                    "Found " + std::to_string(errorCount) + " errors in last "
                    + std::to_string(windowSize_) + " entries",2});

                errorCount = 0;
                window.clear();
            }
        }
        return anamalies;
    }
};

class KeywordDetect: public DetectionStrategy{
private:
    std::vector<std::string> suspicious_;

    std::string toLower(std::string s){
        for(char& c:s) c = tolower(c);
        return s;
    }

public:
    KeywordDetect(){
        suspicious_ = {
            "timeout", "failed", "crash", "null", "overflow",
            "denied", "corrupted", "leak", "segfault", "abort"
        };
    }

    std::string name() const override {return "KeywordDetector";}

    std::vector<Anomaly> analyse(const std::vector<LogEntry>& entries) override{
        std::vector<Anomaly> anomalies;

        std::unordered_map<std::string,int> keywordFreq;

        for(const auto& i:entries){
            std::string lowerMsg = toLower(i.message);

            for(const auto& j:suspicious_){
                if(lowerMsg.find(j) != std::string::npos){
                    keywordFreq[j]++;
                }
            }
        }

        for(const auto& [i,j]:keywordFreq){
            if(j>3){
                anomalies.push_back({
                    "REPEATED_KEYWORD","keyword '"+ i + "' appeared" + std::to_string(j) + " times",
                    j>10 ? 3:2
                });
            }
        }

        return anomalies;
    }
};

class SourceBurst: public DetectionStrategy{
public:
    std::string name() const override {return "SourceBurstDetector"; }

    std::vector<Anomaly> analyse(const std::vector<LogEntry> &entries) override {
        std::vector<Anomaly> anomalies;
        std::unordered_map<std::string,int> sourceCount;

        for(const auto& i:entries){
            sourceCount[i.source]++;
        }

        int total = (int)entries.size();
        for(const auto& [source,count]:sourceCount){
            double prec = (double)count/total *100.0;

            if(prec>60 && total>5){
                anomalies.push_back({
                    "SOURCE_LOG_BURST",
                    "Source '" + source + "' generated " + std::to_string(count)
                        + " entries (" + std::to_string((int)prec) + "% of total)",
                    2
                });
            }
        }
        return anomalies;
    }
};

class AnomalyDetector {
private:
    std::vector<std::unique_ptr<DetectionStrategy>> strategies_;

public:
    AnomalyDetector() {
        // Register all strategies
        // LEARN: std::make_unique = smart pointer (auto memory management)
        // No manual new/delete needed — memory freed automatically!
        strategies_.push_back(std::make_unique<ErrorRate>());
        strategies_.push_back(std::make_unique<KeywordDetect>());
        strategies_.push_back(std::make_unique<SourceBurst>());
    }

    // Run ALL strategies and collect results
    std::vector<Anomaly> detect(const std::vector<LogEntry>& entries) {
        std::vector<Anomaly> allAnomalies;

        for (const auto& strategy : strategies_) {
            std::cout << "[AnomalyDetector] Running: " << strategy->name() << "\n";
            auto found = strategy->analyse(entries);
            // Append found anomalies to master list
            allAnomalies.insert(allAnomalies.end(), found.begin(), found.end());
        }

        return allAnomalies;
    }
};

// class SpikeDetector : public DetectionStrategy