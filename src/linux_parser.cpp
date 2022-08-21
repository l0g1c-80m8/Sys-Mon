#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream basicInputStringStream(line);
      while (basicInputStringStream >> key >> value) {
        if (key == kPrettyName) {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream basicInputStringStream(line);
    basicInputStringStream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
    // https://www.thegeekdiary.com/understanding-proc-meminfo-file-analyzing-memory-utilization-in-linux/
    // https://stackoverflow.com/questions/41224738/how-to-calculate-system-memory-usage-from-proc-meminfo-like-htop/41251290
    // Memory Utilization = MemTotal - MemFree / MemTotal; MemFree = LowFree + HighFree
    string line, key, value;
    float memFree = 1.0f, memTotal = 1.0f; // avoid division by zero
    bool foundMemFree = false, foundMemTotal = false;
    std::ifstream stream(kProcDirectory + kMeminfoFilename);
    if (stream.is_open()) {
        while (std::getline(stream, line)) {
            std::remove(line.begin(), line.end(), ' ');
            std::replace(line.begin(), line.end(), ':', ' ');
            std::istringstream basicInputStringStream(line);
            while (basicInputStringStream >> key >> value && !(foundMemFree && foundMemTotal)) {
                if (key == kMemTotal) {
                    memTotal = stof(value);
                    foundMemTotal = true;
                }
                else if (key == kMemFree) {
                    memFree = stof(value);
                    foundMemFree = true;
                }
            }
        }
    }
    return (memTotal - memFree) / memTotal;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
    string line, value;
    long int uptime = 0;
    std::ifstream stream(kProcDirectory + kUptimeFilename);
    if (stream.is_open()) {
        if (std::getline(stream, line)) {
            std::istringstream basicInputStringStream(line);
            basicInputStringStream >> value;
            uptime = stol(value);
        }
    }
    return uptime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies(vector<long> cpuStats = LinuxParser::CpuUtilization()) {
    return LinuxParser::ActiveJiffies(cpuStats) + LinuxParser::IdleJiffies(cpuStats);
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
    string line, value;
    // READ:
    // (14) utime %lu
    // (15) stime %lu
    // (16) cutime %ld
    // (17) cstime %ld
    // (22) starttime %llu
    // @ https://man7.org/linux/man-pages/man5/proc.5.html
    long utime = 0, stime = 0, cutime = 0, cstime = 0;
    int itr = 0;
     std::ifstream stream(LinuxParser::kProcDirectory + to_string(pid) + LinuxParser::kStatFilename);
    if (stream.is_open()) {
        if (std::getline(stream, line)) {
            std::istringstream basicInputStringStream(line);
            while (itr < 13 && basicInputStringStream >> value) { itr += 1; }
            basicInputStringStream >> utime >> stime >> cutime >> cstime;
        }
    }
    // Active jiffies' calculation: https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
    return utime + stime + cutime + cstime;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies(vector<long> cpuStats = LinuxParser::CpuUtilization()) {
    long activeJiffies = cpuStats[0] + cpuStats[1] + cpuStats[2] + cpuStats[5] + cpuStats[6] + cpuStats[7];
    return activeJiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies(vector<long> cpuStats = LinuxParser::CpuUtilization()) {
    long idleJiffies = cpuStats[3] + cpuStats[4];
    return idleJiffies;
}

// Read and return CPU stats to be used for calculating CPU utilization
vector<long> LinuxParser::CpuUtilization() {
    string line, key, user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    bool foundCpuStats = false;
    std::ifstream filestream(kProcDirectory + kStatFilename);
    if (filestream.is_open()) {
        while (getline(filestream, line) && !foundCpuStats) {
            std::istringstream stream(line);
            while (stream >> key) {
                if (key == kCpuStats) {
                    foundCpuStats = true;
                    // See below for order of tokens {user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice}:
                    // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
                    stream >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
                    break;
                }
            }
        }
    }
    return vector<long> {
        stol(user),
        stol(nice),
        stol(system),
        stol(idle),
        stol(iowait),
        stol(irq),
        stol(softirq),
        stol(steal),
        stol(guest),
        stol(nice)
    };
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
    string line, key, value;
    long int totalProcesses = 0;
    bool foundTotalProcesses = false;
    std::ifstream stream(kProcDirectory + kStatFilename);
    if (stream.is_open()) {
        while (std::getline(stream, line) && !foundTotalProcesses) {
            std::istringstream basicInputStringStream(line);
            while (basicInputStringStream >> key >> value) {
                if (key == kTotalProcesses) {
                    totalProcesses = stol(value);
                    foundTotalProcesses = true;
                    break;
                }
            }
        }
    }
    return totalProcesses;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
    string line, key, value;
    long int runningProcesses = 0;
    bool foundRunningProcesses = false;
    std::ifstream stream(kProcDirectory + kStatFilename);
    if (stream.is_open()) {
        while (std::getline(stream, line) && !foundRunningProcesses) {
            std::istringstream basicInputStringStream(line);
            while (basicInputStringStream >> key >> value) {
                if (key == kRunningProcesses) {
                    runningProcesses = stol(value);
                    foundRunningProcesses = true;
                    break;
                }
            }
        }
    }
    return runningProcesses;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid[[maybe_unused]]) {
    string line;
    string command{""};
    std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
    if (stream.is_open()) {
        if (std::getline(stream, line)) {
            std::istringstream basicInputStringStream(line);
            basicInputStringStream >> command;
        }
    }
    return command;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
    string line, key;
    string ram{"0"};
    bool foundRam = false;
    std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
    if(stream.is_open()) {
        while (std::getline(stream, line) && !foundRam) {
            std::istringstream basicInputStringStream(line);
            while (basicInputStringStream >> key) {
                if (key == kProcMemSize) {
                    basicInputStringStream >> ram;
                    foundRam = true;
                    break;
                }
            }
        }
    }
    return ram;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid[[maybe_unused]]) {
    string line, key;
    string uid{"-1"};
    bool foundUid = false;
    std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
    if (stream.is_open()) {
        while (std::getline(stream, line) && !foundUid) {
            std::istringstream basicInputStringStream(line);
            while (basicInputStringStream >> key) {
                if (key == kProcUid) {
                    basicInputStringStream >> uid;
                    foundUid = true;
                    break;
                }
            }
        }
    }
    return uid;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
    string line, value;
    string uid = LinuxParser::Uid(pid);
    string user{""};
    bool foundUser = false;
    std::ifstream stream(kPasswordPath);
    if (stream.is_open()) {
        while (std::getline(stream, line) && !foundUser) {
            std::replace(line.begin(), line.end(), ':', ' ');
            std::replace(line.begin(), line.end(), 'x', ' ');
            std::istringstream basicInputStringStream(line);
            while (basicInputStringStream >> user >> value) {
                if (value == uid) {
                    foundUser = true;
                    break;
                }
            }
        }
    }
    return user;
}

// Read and return the uptime of a process
float LinuxParser::UpTime(int pid) {
    string line, value;
    string startTime{"0"};
    int itr = 0;
    std::ifstream stream(LinuxParser::kProcDirectory + to_string(pid) + LinuxParser::kStatFilename);
        // READ: (22) starttime %llu @ https://man7.org/linux/man-pages/man5/proc.5.html
        if (stream.is_open()) {
            if (std::getline(stream, line)) {
                std::istringstream basicInputStringStream(line);
                while (itr < 22 && basicInputStringStream >> value) { itr += 1; }
                startTime = value;
            }
        }
    return stol(startTime) / sysconf(_SC_CLK_TCK);
}
