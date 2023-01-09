#include "simulator.h"


int Simulator::allJobsDone = 0;
int Simulator::numIteration = 0;
mutex Simulator::tableMutex;
ResourceManager* Simulator::manager;
map<pthread_t, JobInfo*, greater<unsigned long>> Simulator::jobs;
unordered_map<pthread_t, mutex> Simulator::statusMutex;


Simulator::Simulator(char *filename, int monitorTime, int numIter) {
    this->file = fopen(filename, "r");

    if (this->file == nullptr) {
        cerr << "fopen() error" << endl;
        exit(1);
    }

    Simulator::numIteration = numIter;
    Simulator::manager = ResourceManager::getInstance();

    if (pthread_create(&monitorId, nullptr, (ThreadFuncPtr)&Monitor::start, (void *)&monitorTime) != 0) {
        cerr << "pthread_create() error" << endl;
    }
}


void Simulator::start() {
    struct timespec start = {0, 0}, end = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &start);

    vector<string> tokens;

    int flag = getSplitLine(this->file, tokens);

    while(flag != EOF) {
        if (flag) {
            parsed(tokens);
        }

        flag = getSplitLine(this->file, tokens);
    }

    void * status;

    for (auto &job : Simulator::jobs) {
        if (pthread_join(job.first, &status) != 0)
            cerr << "pthread_join error()" << endl;
    }

    Simulator::allJobsDone = 1;

    if (pthread_join(monitorId, &status) != 0)
        cerr << "pthread_join error()" << endl;

    printJob();

    clock_gettime(CLOCK_MONOTONIC, &end);

    cout << "Running time= ";
    cout << 1000UL * end.tv_sec + end.tv_nsec / 1000000UL - (1000UL * start.tv_sec + start.tv_nsec / 1000000UL)
         << "msec" << endl;

    fclose(this->file);

}


void Simulator::printJob() {
    cout << "System Jobs" << endl;

    int index = 0;
    string status;

    for (auto &job: Simulator::jobs) {
        switch(job.second->status) {
            case WAIT:
                status = "WAIT";
                break;
            case RUN:
                status = "RUN";
                break;
            case IDLE:
                status = "IDLE";
                break;
        }

        cout << "[" << index++ << "] " << job.second->jobName << " (" << status;
        cout << ", " << "runTime= " << job.second->busyTime << ", idleTime= " << job.second->idleTime << " msec):"
             << endl;
        cout << "        (tid= " << job.first << ")" << endl;
        for (auto &rsrc: job.second->need)
            cout << "        " << rsrc.first << ": (need= " << rsrc.second << ", held= "
                 << job.second->held[rsrc.first] << ")" << endl;
        cout << "(RUN: " << job.second->currentIteration << " times, WAIT: " << job.second->waitTime << " msec)"
             << endl << endl;
    }
}


void Simulator::parsed(vector<string> &tokens) {
    if (tokens.front() == "resources") {
        for (auto it = tokens.begin() + 1; it != tokens.end(); ++it) {
            if (Simulator::manager->addResource(*it))
                cerr << "maximum number of resource types reaches" << endl;
        }
    } else if (tokens.front() =="job")
        createJob(tokens);
}


void Simulator::createJob(vector<string> &args) {
    if (numJobs >= NJOBS) {
        cerr << "Maximum number of jobs reached" << endl;
        return;
    }

    JobInfo *info = Job::composeJobInfo(args);

    pthread_t jobId;

    if (pthread_create(&jobId, nullptr, (ThreadFuncPtr)&Job::start, (void *)info) != 0) {
        cerr << "pthread_create() error" << endl;
    } else {
        Simulator::jobs[jobId] = info;
        Simulator::statusMutex[jobId];
        numJobs++;
    }
}


void * Job::start(void *arg) {
    unsigned long runtime = 0;
    const unsigned waitTime = (5 % 1000) * 1000;

    struct timespec start = {0, 0}, end = {0, 0};
    struct timespec startWait = {0, 0}, endWait = {0, 0};
    struct timeval tv = {0, 0};

    auto *info = (JobInfo *)arg;

    while (info->currentIteration < Simulator::numIteration) {
        if (info->status != WAIT) {
            lock_guard<mutex> lock(Simulator::statusMutex[pthread_self()]);

            info->status = WAIT;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        clock_gettime(CLOCK_MONOTONIC, &startWait);

        for (;;) {
            Simulator::tableMutex.lock();
            if (checkAllResource(info)) {
                Simulator::tableMutex.unlock();

                tv.tv_usec = waitTime;
                select(0, nullptr, nullptr, nullptr, &tv);
            } else {
                getAllResource(info);

                Simulator::tableMutex.unlock();

                break;
            }
        }

        {
            lock_guard<mutex> lock(Simulator::statusMutex[pthread_self()]);

            info->status = RUN;

            tv.tv_usec = (info->busyTime % 1000) * 1000;
            select(0, nullptr, nullptr, nullptr, &tv);
        }

        clock_gettime(CLOCK_MONOTONIC, &endWait);
        info->waitTime += 1000UL * endWait.tv_sec + endWait.tv_nsec / 1000000UL -
                (1000UL * startWait.tv_sec + startWait.tv_nsec / 1000000UL);

        returnAllResource(info);

        {
            lock_guard<mutex> lock(Simulator::statusMutex[pthread_self()]);

            info->status = IDLE;

            tv.tv_usec = (info->busyTime % 1000) * 1000;
            select(0, nullptr, nullptr, nullptr, &tv);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        runtime += 1000UL * end.tv_sec + end.tv_nsec / 1000000UL -
                (1000UL * start.tv_sec + start.tv_nsec / 1000000UL);

//        lock_guard<mutex> lock(Simulator::outputMutex);

        info->currentIteration++;

        cout << "job: " << info->jobName << " (tid= " << pthread_self() << ", iter=" << info->currentIteration <<
        ", time= " << runtime << " msec)" << endl;
    }

    return nullptr;
}


int Job::checkAllResource(JobInfo *info) {
    int numUnits = 0;
    string type;

    for (auto &need: info->need) {
        type = need.first;
        numUnits = need.second;

        if (numUnits > Simulator::manager->getResource(type))
            return 1;
    }

    return 0;
}

void Job::getAllResource(JobInfo *info) {
    int numUnits = 0;
    string type;

    for (auto &need: info->need) {
        type = need.first;
        numUnits = need.second;

        Simulator::manager->getResource(type, numUnits);
        info->held[type] = numUnits;
    }
}


void Job::returnAllResource(JobInfo *info) {
    int numUnits = 0;
    string type;

    lock_guard<mutex> lock(Simulator::tableMutex);

    for (auto &held: info->held) {
        type = held.first;
        numUnits = held.second;

        Simulator::manager->returnResource(type, numUnits);
        held.second = 0;
    }
}


JobInfo *Job::composeJobInfo(vector<string> &args) {
    auto *info = new JobInfo();

    auto it = args.begin() + 1;

    info->jobName = *(it++);

    info->busyTime = stoi(*it++);
    info->idleTime = stoi(*it++);
    info->status = WAIT;

    for (;it != args.end(); ++it) {
        unsigned long index = it->find(':');

        string type = it->substr(0, index);
        string numUnits = it->substr(index + 1);

        info->need[type] = stoi(numUnits);
        info->held[type] = 0;
    }

    return info;
}


void * Monitor::start(void *arg) {
    int *monitorTime = (int *)arg;

    struct timeval tv = {0, (*monitorTime % 1000) * 1000};

    string wait = "[WAIT]";
    string run = "[RUN]";
    string idle = "[IDLE]";

    while (!Simulator::allJobsDone) {

        wait = "[WAIT]";
        run = "[RUN]";
        idle = "[IDLE]";

        for (auto &m: Simulator::statusMutex)
            m.second.lock();

        for (auto &job: Simulator::jobs) {
            string jobName = " " + job.second->jobName;
            Status status = job.second->status;

            switch (status) {
                case WAIT:
                    wait += jobName;
                    break;
                case RUN:
                    run += jobName;
                    break;
                case IDLE:
                    idle += jobName;
            }
        }

        cout << "monitor: " << wait << endl;
        cout << "         " << run << endl;
        cout << "         " << idle << endl;
        cout << "..." << endl;

        for (auto &m: Simulator::statusMutex)
            m.second.unlock();

        select(0, nullptr, nullptr, nullptr, &tv);
    }

    cout << endl;
    return nullptr;
}
