#include "processor.h"
#include "linux_parser.h"
#include <vector>

using std::vector;
using std::string;

Processor::Processor() : prevTotalTime(0.0), prevIdleTime(0.0) {};

// convert the given string vector into a long vector
vector<long> Processor::GetVectorizedUtilizations(vector<string> values) {
    vector<long> utilizations{};

    for (int idx = 0; idx < (int)values.size(); idx++) {
        try {
            utilizations.push_back(std::stol(values[idx]));
        } catch (const std::invalid_argument& arg) {
            utilizations.push_back((long) 0);
        }
    }
    return utilizations;
}

// Return the aggregate CPU utilization
float Processor::Utilization() {
    vector<long> cpuValues = GetVectorizedUtilizations(LinuxParser::CpuUtilization());
    float totalTime =
            cpuValues[LinuxParser::kUser_] + cpuValues[LinuxParser::kNice_] + cpuValues[LinuxParser::kSystem_] + cpuValues[LinuxParser::kIdle_] + cpuValues[LinuxParser::kIOwait_] + cpuValues[LinuxParser::kIRQ_] +  cpuValues[LinuxParser::kSoftIRQ_] + cpuValues[LinuxParser::kSteal_];
    float idleTime =
            cpuValues[LinuxParser::kIOwait_] + cpuValues[LinuxParser::kIdle_];

    float deltaTotal = totalTime - prevTotalTime;
    float deltaIdle = idleTime - prevIdleTime;
    float usage = (deltaTotal - deltaIdle) / deltaTotal;

    prevIdleTime = idleTime;
    prevTotalTime = totalTime;

    return usage;
}