#ifndef UTILS_H
#define UTILS_H


#include <cstring>
#include <cstdarg>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#define BUFFER_SIZE 64

using namespace std;


int getSplitLine(FILE *, vector<string> &);

int isComment(char *);

int split(string, string, vector<string> &);


#endif