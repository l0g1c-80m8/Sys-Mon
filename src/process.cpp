#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid) {
    command_ = LinuxParser::Command(Pid());
    user_ = LinuxParser::User(Pid());
    uptime_ = LinuxParser::UpTime(Pid());

    string usage = LinuxParser::Ram(Pid());
    try {
        float mbUsage = std::stol(usage) / 1024;
        ram_ = std::to_string(mbUsage);
    } catch (const std::invalid_argument& arg) {
        ram_ = "0";
    }

    long uptime = LinuxParser::UpTime();
    vector<float> val = LinuxParser::CpuUtilization(Pid());
    // if all values were read
    if (val.size() == 5) {
        // add utime, stime, cutime, cstime (in seconds)
        float totaltime = val[kUtime_] + val[kStime_] + val[kCutime_] + val[kCstime_];
        float seconds = uptime - val[kStarttime_];
        cpu_ = totaltime / seconds;
    } else
        cpu_ = 0.0f;
}

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const { return cpu_; }

// Return the command that generated this process
string Process::Command() { return command_; }

// Return this process's memory utilization
string Process::Ram() { return ram_; }

// Return the user (name) that generated this process
string Process::User() { return user_; }

// Return the age of this process (in seconds)
long int Process::UpTime() { return uptime_; }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
    return CpuUtilization() > a.CpuUtilization();
}