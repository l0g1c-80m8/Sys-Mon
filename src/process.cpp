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

Process::Process(int pid): pid(pid) {
    user = LinuxParser::User(pid);
    ram = to_string(stol(LinuxParser::Ram(pid)) / 1024); // convert from KB to MB
    upTime = LinuxParser::UpTime(pid); // process uptime in seconds; See LinuxParser::UpTime(int pid);
    command = LinuxParser::Command(pid);

    // Final CPU Utilization Calculation:
    // https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
    long activeJiffies = LinuxParser::ActiveJiffies(pid);
    cpu = (float) activeJiffies / (sysconf(_SC_CLK_TCK) * upTime);
}

// Return this process's ID
int Process::Pid() { return pid; }

// Return this process's CPU utilization
float Process::CpuUtilization() const { return cpu; }

// Return the command that generated this process
string Process::Command() { return command; }

// Return this process's memory utilization
string Process::Ram() { return ram; }

// Return the user (name) that generated this process
string Process::User() { return user; }

// Return the age of this process (in seconds)
long int Process::UpTime() { return upTime; }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& proc) const {
    return CpuUtilization() > proc.CpuUtilization();
}