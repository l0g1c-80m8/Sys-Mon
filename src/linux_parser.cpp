#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
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
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
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
  float memTotal = 0.0f, memFree = 0.0f;
  string line, key, value;

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::remove(line.begin(), line.end(), ' ');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") {
          memTotal = std::stof(value);
        }
        else if (key == "MemFree") {
          memFree = std::stof(value);
          // MemTotal is always first, break when both are read
          break;
        }
      }
    }
  }

  return ((memTotal - memFree) / memTotal);
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string line, upTime;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> upTime) {
        try {
          return std::stol(upTime);
        } catch (const std::invalid_argument& arg) {
          return 0;
        }
      }
    }
  }
  return 0;
}

// Not needed - all read in LinuxParser::CpuUtilization()
// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// Not needed
// Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// Not needed - all read in LinuxParser::CpuUtilization()
// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// Not needed - all read in LinuxParser::CpuUtilization()
// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// Not needed - all read in LinuxParser::CpuUtilization()
// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
    vector<string> cpuValues{};
    string line, key;
    string vuser, vnice, vsystem, vidle, viowait, virq, vsoftirq, vsteal, vguest, vguest_nice;

    std::ifstream filestream(kProcDirectory + kStatFilename);
    if (filestream.is_open()) {
        while (std::getline(filestream, line)) {
            std::istringstream linestream(line);
            while (linestream >> key >> vuser >> vnice >> vsystem >> vidle >>
                              viowait >> virq >> vsoftirq >> vsteal >> vguest >> vguest_nice) {
                if (key == "cpu") {
                    cpuValues.push_back(vuser);
                    cpuValues.push_back(vnice);
                    cpuValues.push_back(vsystem);
                    cpuValues.push_back(vidle);
                    cpuValues.push_back(viowait);
                    cpuValues.push_back(virq);
                    cpuValues.push_back(vsoftirq);
                    cpuValues.push_back(vsteal);
                    cpuValues.push_back(vguest);
                    cpuValues.push_back(vguest_nice);
                    return cpuValues;
                }
            }
        }
    }
    return {};
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, key, value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          try {
            // string to long int - https://cplusplus.com/reference/string/stol/
            return std::stol(value);
          } catch (const std::invalid_argument& arg) {
            return 0;
          }
        }
      }
    }
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line, key, value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          try {
            // string to long int - https://cplusplus.com/reference/string/stol/
            return std::stol(value);
          } catch (const std::invalid_argument& arg) {
            return 0;
          }
        }
      }
    }
  }
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
    string value = "";
    std::ifstream filestream(kProcDirectory + "/" + std::to_string(pid) + kCmdlineFilename);
    if (filestream.is_open()) {
        std::getline(filestream, value);
        return value;
    }
    return value;
}

// Read and return the CPU usage of a process (in seconds)
// Overloaded for a single process
vector<float> LinuxParser::CpuUtilization(int pid) {
    vector<float> cpuValues{};
    string line, value;
    float time = 0.0f;

    std::ifstream filestream(kProcDirectory + "/" + std::to_string(pid) + kStatFilename);
    if (filestream.is_open()) {
        while (std::getline(filestream, line)) {
            std::istringstream linestream(line);
            for (int i = 1; i <= kStarttime_; i++) {
                linestream >> value;
                if (i == kUtime_ || i == kStime_ || i == kCutime_ || i == kCstime_ || i == kStarttime_) {
                    // jiffies / rate (jiffies per second); jiffies AKA clock ticks
                    time = std::stof(value) / sysconf(_SC_CLK_TCK);
                    cpuValues.push_back(time);
                }
            }
        }
    }
    return cpuValues;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
    string line, key, value = "";
    std::ifstream filestream(kProcDirectory + "/" + std::to_string(pid) + kStatusFilename);
    if (filestream.is_open()) {
        while (std::getline(filestream, line)) {
            std::replace(line.begin(), line.end(), ':', ' ');
            std::istringstream linestream(line);
            while (linestream >> key >> value) {
                if (key == "VmSize") {
                    return value;
                }
            }
        }
    }
    return value;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
    string line, key, value = "";
    std::ifstream filestream(kProcDirectory + "/" + std::to_string(pid) + kStatusFilename);
    if (filestream.is_open()) {
        while (std::getline(filestream, line)) {
            std::replace(line.begin(), line.end(), ':', ' ');
            std::istringstream linestream(line);
            while (linestream >> key >> value) {
                if (key == "Uid") {
                    return value;
                }
            }
        }
    }
    return value;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
    string uid = Uid(pid), line, key, value = "", dump;
    std::ifstream filestream(kPasswordPath);
    if (filestream.is_open()) {
        while (std::getline(filestream, line)) {
            std::replace(line.begin(), line.end(), ':', ' ');
            std::istringstream linestream(line);
            while (linestream >> value >> dump >> key) {
                if (key == uid) {
                    return value;
                }
            }
        }
    }
    return value;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
    string line, value = "";
    long uptime = 0;
    std::ifstream filestream(kProcDirectory + "/" + std::to_string(pid) +
                             kStatFilename);
    if (filestream.is_open()) {
        while (std::getline(filestream, line)) {
            std::istringstream linestream(line);
            for (int i = 1; i <= kStarttime_; i++) {
                linestream >> value;
                if (i == kStarttime_) {
                    try { // jiffies / jiffies per second = seconds
                        uptime = std::stol(value) / sysconf(_SC_CLK_TCK);
                        return uptime;
                    } catch (const std::invalid_argument& arg) {
                        return 0;
                    }
                }
            }
        }
    }
    return uptime;
}