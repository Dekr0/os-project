#ifndef UTILS_H
#define UTILS_H


#include <cstdarg>
#include <memory>
#include <string>
#include <vector>


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