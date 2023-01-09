#ifndef UTILS_H
#define UTILS_H


#include <cstring>
#include <cstdarg>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>

#include "handler.h"


using namespace std;


typedef void * (*THREADFUNCPTR) (void *);

int stoSwNum (string str);

void mkopen ( int s1, int s2, int &rfd, int &wfd );

void mkopenr ( int s1, int s2, int &rfd );

void mkopenw ( int s1, int s2, int &rfd );

int split ( string str, string del, vector<string> &tokens );

// ssize_t readn ( int fd, void *ptr, size_t n );

// ssize_t writen ( int fd, const void *ptr, size_t n );


#endif