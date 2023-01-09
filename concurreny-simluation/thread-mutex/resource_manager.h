#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H


#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

#define NRES_TYPES 10

using namespace std;


typedef struct {
    int max;
    int held;
} UnitsCount;


class ResourceManager {

protected:
    ResourceManager() {};

    ~ResourceManager() {};

public:
    unordered_map<string, mutex> semaphores;

    static ResourceManager *getInstance();

    ResourceManager(ResourceManager &other) = delete;

    void operator=(const ResourceManager &) = delete;

    int addResource(const string&);
    int getResource(const string&);
    void getResource(const string&, int);
    void returnResource(const string&, int);
    void printResource();

private:
    static ResourceManager *manager;
    static mutex mutex_;

    unordered_map<string, UnitsCount> resources;

    int numTypes;

};


#endif
