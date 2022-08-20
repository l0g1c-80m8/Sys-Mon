#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <vector>
#include <fstream>
#include <regex>
#include <string>

using std::vector;

namespace LinuxParser {
// Paths
const std::string kProcDirectory{"/proc/"};
const std::string kCmdlineFilename{"/cmdline"};
const std::string kCpuinfoFilename{"/cpuinfo"};
const std::string kStatusFilename{"/status"};
const std::string kStatFilename{"/stat"};
const std::string kUptimeFilename{"/uptime"};
const std::string kMeminfoFilename{"/meminfo"};
const std::string kVersionFilename{"/version"};
const std::string kOSPath{"/etc/os-release"};
const std::string kPasswordPath{"/etc/passwd"};

// System
float MemoryUtilization();
long UpTime();
std::vector<int> Pids();
int TotalProcesses();
int RunningProcesses();
std::string OperatingSystem();
std::string Kernel();

// CPU
enum CPUStates {
  kUser_ = 0,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_
};
std::vector<long> CpuUtilization();
long Jiffies(vector<long> cpuStats);
long ActiveJiffies(vector<long> cpuStats);
long ActiveJiffies(int pid);
long IdleJiffies(vector<long> cpuStats);

// Processes
std::string Command(int pid);
std::string Ram(int pid);
std::string Uid(int pid);
std::string User(int pid);
long int UpTime(int pid);

// File Keys
const std::string kPrettyName{"PRETTY_NAME"};
const std::string kMemFree{"MemFree"};
const std::string kMemTotal{"MemTotal"};
const std::string kTotalProcesses{"processes"};
const std::string kRunningProcesses{"procs_running"};
const std::string kCpuStats{"cpu"};
const std::string kProcUid{"Uid:"};
const std::string kProcMemSize{"VmSize:"};
};  // namespace LinuxParser

#endif