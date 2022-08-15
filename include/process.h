#ifndef PROCESS_H
#define PROCESS_H

#include <string>

using std::string;

/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
    Process(int);
    int Pid();                               // See src/process.cpp
    std::string User();                      // See src/process.cpp
    std::string Command();                   // See src/process.cpp
    float CpuUtilization() const;                  // See src/process.cpp
    std::string Ram();                       // See src/process.cpp
    long int UpTime();                       // See src/process.cpp
    bool operator<(Process const&) const;  // See src/process.cpp

// Declare any necessary private members
 private:
    int pid_;
    string command_;
    string user_;
    long uptime_;
    string ram_;
    float cpu_;
    enum ProcessStates {
        kUtime_ = 0,
        kStime_,
        kCutime_,
        kCstime_,
        kStarttime_
    };
};

#endif