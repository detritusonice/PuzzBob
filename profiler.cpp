#include "base.h"
#include "profiler.h"

#include <chrono>
#include <cstring>
#include <cstdio>

Profiler::Profiler()
{

    programStart = std::chrono::high_resolution_clock::now();
    std::chrono::system_clock::time_point tp = std::chrono::high_resolution_clock::now();

    timeVector.push_back(0ll);
    timeVector[0] -=  std::chrono::duration_cast< std::chrono::microseconds> (tp-programStart).count();
    nameVector.push_back("Total Time");
    lastHandle=0;
    indexMap.clear();
    usageVector.push_back(0);

    std::string fname(__func__);

    classNameIncluded = (fname.find("::")!=std::string::npos );

}

Profiler::~Profiler()
{
    writeResults();
    timeVector.clear();
    nameVector.clear();
    indexMap.clear();
}

void Profiler::startRec(std::string className, std::string functionName)
{
    std::string name;

    if (!classNameIncluded)
        name=className+"::"+functionName;
    else
        name=functionName;

    int handle= indexMap[name];

    if (handle==0)//non existent;
    {
        lastHandle++;
        timeVector.push_back(0ll);
        usageVector.push_back(0);
        nameVector.push_back(name);
        indexMap[name]=lastHandle;
        handle=lastHandle;
    }
    std::chrono::system_clock::time_point tp = std::chrono::high_resolution_clock::now();

    timeVector[handle]-=std::chrono::duration_cast< std::chrono::microseconds> (tp-programStart).count();
    usageVector[handle]+=1;
}

void Profiler::stopRec(std::string className, std::string functionName)
{
    std::string name;

    if (!classNameIncluded)
        name=className+"::"+functionName;
    else
        name=functionName;

    int handle = indexMap[name];

    if (handle==0) return;

    std::chrono::system_clock::time_point tp = std::chrono::high_resolution_clock::now();

    timeVector[handle]+=std::chrono::duration_cast< std::chrono::microseconds> (tp-programStart).count();
}

void Profiler::writeResults()
{

    char filename[500];

    sprintf(filename,"%s/profiling.txt",glbAppPath);

    SDL_RWops *out;

    out = SDL_RWFromFile(filename,"wt");

    if (!out)
        return;

    std::chrono::system_clock::time_point tp = std::chrono::high_resolution_clock::now();

    timeVector[0]+=std::chrono::duration_cast< std::chrono::microseconds> (tp-programStart).count();

    char buffer[200];
    double totalTime,time_perc;

    totalTime=timeVector[0];

    sprintf(buffer,"%40s === ",nameVector[0].c_str());
    SDL_RWwrite(out,buffer,sizeof(char),strlen(buffer));

    sprintf(buffer,"%.2f seconds\r\n",totalTime/1000000.);
    SDL_RWwrite(out,buffer,sizeof(char),strlen(buffer));

    sprintf(buffer,"%40s\t","ClassName::MemberName");
    SDL_RWwrite(out,buffer,sizeof(char),strlen(buffer));

    sprintf(buffer,"Time %%\tCalls\tDuration\r\n");
    SDL_RWwrite(out,buffer,sizeof(char),strlen(buffer));


    for (int i=1;i<timeVector.size();i++)
    {

        time_perc=timeVector[i]/totalTime;
        time_perc*=100.;
        std::string s=nameVector[i];
        int p1=s.find_first_of('(');

        sprintf(buffer,"%40s\t",s.substr(0,p1).c_str());
        SDL_RWwrite(out,buffer,sizeof(char),strlen(buffer));

        sprintf(buffer,"%02.4f\t",time_perc);
        SDL_RWwrite(out,buffer,sizeof(char),strlen(buffer));

        sprintf(buffer,"%8d\t",usageVector[i]);
        SDL_RWwrite(out,buffer,sizeof(char),strlen(buffer));

        sprintf(buffer,"%5d.%06d\r\n",(int)(timeVector[i]/1000000ll),(int)(timeVector[i]%1000000ll));
        SDL_RWwrite(out,buffer,sizeof(char),strlen(buffer));

        if (i==2)
            totalTime=timeVector[2];//counts the time the app was active in the processor
    }

    SDL_RWclose(out);


}

