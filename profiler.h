#ifndef PROFILER_H
#define PROFILER_H


#include <vector>
#include <map>
#include <string>
#include <chrono>

class Profiler
{
    int lastHandle;
    bool classNameIncluded;

    std::vector<int> usageVector;

    std::vector<long long> timeVector;
    std::vector<std::string> nameVector;

    std::map<std::string,int> indexMap;
    std::chrono::system_clock::time_point programStart;

public:
    Profiler();
    ~Profiler();
    void startRec(std::string className, std::string functionName);
    void stopRec(std::string className, std::string functionName);

    void writeResults();
};





#endif // PROFILER_H
