#include <string>

#include "format.h"

using std::string;

std::string Format::FullTimeFormat(long time) {
    if (time < 10)
        return "0" + std::to_string(time);
    return std::to_string(time);
}

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
    string time = "00:00:00";
    if (seconds <= 0)
        return time;
    long rmdr = 0;
    rmdr = seconds / 3600;
    time = FullTimeFormat(rmdr) + ":";
    rmdr = (seconds / 60) % 60;
    time += FullTimeFormat(rmdr) + ":";
    rmdr = seconds % 60;
    time += FullTimeFormat(rmdr);
    return time;
}