#ifndef SIMULATOR_H
#define SIMULATOR_H


#include <cmath>
#include <cstdio>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

#include "pthread.h"
#include "unistd.h"

#include "resource_manager.h"
#include "utils.h"

#define NJOBS 25

using namespace std;

typedef enum {WAIT, RUN, IDLE} Status;

typedef struct {
    int busyTime;
    int currentIteration = 0;
    int idleTime;
    unsigned long waitTime;
    string jobName;
    Status status;
    map<string, int> need;
    map<string, int> held;
} JobInfo;


typedef void * (*ThreadFuncPtr)(void *);


class Simulator {

public:
    static int numIteration;
    static int allJobsDone;
    static mutex tableMutex;
    static ResourceManager *manager;
    static map<pthread_t, JobInfo*, greater<unsigned long>> jobs;
    static unordered_map<pthread_t, mutex> statusMutex;

    Simulator(char *, int, int);

    void start();
    void printJob();

private:
    int numJobs = 0;
    pthread_t monitorId;

    FILE *file;

    void parsed(vector<string> &);
    void createJob(vector<string> &);
};


class Job {

public:
    static void * start(void *);

    static int checkAllResource(JobInfo *);
    static void getAllResource(JobInfo *);
    static void returnAllResource(JobInfo *);

    static JobInfo *composeJobInfo(vector<string> &);
};


class Monitor {

public:
    static void * start(void *);

};


#endif
