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

void split(const string&, const string&, vector<string> &);

string strip(string);

void strip(char *);

#endif