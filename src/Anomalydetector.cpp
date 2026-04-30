#pragma once
#include "LogEntry.cpp"
#include <vector>
#include <string>

struct Anomaly {
    std::string type;
    std::string detail;
    int         severity;
};

/*
Abstract Base Class -->
class that cannot be intialized
Just an blueprint/interface
*/

class DetectionStrategy{
    public:

};