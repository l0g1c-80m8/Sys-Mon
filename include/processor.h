#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <string>
#include <vector>

using std::vector;
using std::string;

class Processor {
 public:
    Processor();
    float Utilization();  // See src/processor.cpp

  // Declare any necessary private members
 private:
    float prevTotalTime;
    float prevIdleTime;

    vector<long> GetVectorizedUtilizations(vector<string> values);
};

#endif