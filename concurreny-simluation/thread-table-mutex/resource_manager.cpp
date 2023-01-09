#include "resource_manager.h"


ResourceManager* ResourceManager::manager = nullptr;
mutex ResourceManager::mutex_;


ResourceManager *ResourceManager::getInstance() {
    lock_guard<mutex> lock(mutex_);
    if (manager == nullptr) {
        manager = new ResourceManager();
    }

    return manager;
}


int ResourceManager::addResource(const string& pairs) {
    if (numTypes >= NRES_TYPES)
        return 1;

    unsigned long index = pairs.find(':');
    string type = pairs.substr(0, index);
    string numUnits = pairs.substr(index + 1);

    UnitsCount count = {stoi(numUnits), 0};
    resources[type] = count;

    numTypes++;

    return 0;
}


int ResourceManager::getResource(const string& type) {
    return resources[type].max;
}


void ResourceManager::getResource(const string& type, int need) {
    resources[type].max -= need;
    resources[type].held += need;
}


void ResourceManager::returnResource(const string& type, int held) {
    resources[type].max += held;
    resources[type].held -= held;
}


void ResourceManager::printResource() {
    cout << "System Resources:" << endl;
    for (auto &rsrc: resources) {
        cout << "\t" << rsrc.first << ": (maxAvail=   " << rsrc.second.max << ", held=   " << rsrc.second.held << ")"
        << endl;
    }
}