#pragma once
#include "LogEntry.cpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <deque>
#include <functional>
#include <iostream>

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
                
            }
        }
    }

};