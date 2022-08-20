#include "processor.h"
#include "linux_parser.h"

using std::string;
using std::vector;

Processor::Processor() : activeJiffies(0), idleJiffies(0) {};

// Return the aggregate CPU utilization
float Processor::Utilization() {
    vector<long> cpuStats = LinuxParser::CpuUtilization();

    // https://stackoverflow.com/a/23376195/16112875

    // add a 1 to avoid zero division
    long updatedActiveJiffies = LinuxParser::ActiveJiffies(cpuStats);
    long updatedIdleJiffies = LinuxParser::IdleJiffies(cpuStats);

    long deltaActive = updatedActiveJiffies - activeJiffies;
    long deltaIdle = updatedIdleJiffies - idleJiffies;

    activeJiffies = updatedActiveJiffies;
    idleJiffies = updatedIdleJiffies;

    // return fraction, % calculated in ncurses_display.cpp
    return (float) deltaActive / (deltaActive + deltaIdle);;
}