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
#include "utils.h"

using namespace std;


int stoSwNum (string str) {
    if ( str.compare("null") == 0 )
        return -1;

    return stoi( str.substr( string("psw").size() ) );
}

void mkopen ( int s1, int s2, int &rfd, int &wfd ) {
    mkopenr ( s1, s2, rfd );
    mkopenw ( s2, s1, wfd );
}

void mkopenr ( int s1, int s2, int &rfd ) {
    string name = "fifo-" + to_string(s1) + "-" + to_string(s2);

    cout << "making " + name + "..." << endl;
    if ( mkfifo( name.c_str(), S_IRWXU ) < 0 ) {
        if ( errno != EEXIST ) {
            fatal( "mkfifo error()" );
            exit(1);
        }
    }
    cout << name + " is made" << endl << endl;

    cout << "opening " + name + " for read..." << endl;
    if ( ( rfd = open( name.c_str(), O_RDWR ) ) < 0 ) {
        fatal( "open error() : %d", errno );
        exit(1);
    }
    cout << name + " is open" << endl << endl;
}

void mkopenw ( int s1, int s2, int &wfd ) {
    string name = "fifo-" + to_string(s1) + "-" + to_string(s2);

    cout << "making " + name + "..." << endl;
    if ( mkfifo( name.c_str(), S_IRWXU ) < 0 ) {
        if ( errno != EEXIST ) {
            fatal( "mkfifo error()" );
            exit(1);
        }
    }
    cout << name + " is made" << endl << endl;

    cout << "opening " + name + " for write..." << endl;
    if ( ( wfd = open( name.c_str(), O_RDWR ) ) < 0 ) {
        fatal( "open error() : %d", errno );
        exit(1);
    }
    cout << name + " is open" << endl << endl;
}

int split ( string str, string del, vector<string> &tokens ) {
    int num = 0;
    int start = 0;
    int end = str.find(del);

    while (end != -1) {
        tokens.push_back( str.substr( start, end - start ) );
        start = end + del.size();
        end = str.find( del, start );
    }

    tokens.push_back( str.substr( start, end - start ) );

    return num;
}

// ssize_t readn ( int fd, void *ptr, size_t n ) {
//     size_t nleft;
//     ssize_t nread;

//     nleft = n;
//     while ( nleft > 0 ) {
//         if ( ( nread = read( fd, ptr, nleft ) ) < 0 ) {
//             if ( nleft == n )
//                 return -1;
//             else
//                 break;
//         } else if ( nread == 0 ) {
//             break;
//         }
//         nleft -= nread;
//         ptr += nread;
//     }

//     return ( n- nleft );
// }

// ssize_t writen ( int fd, const void *ptr, size_t n ) {
//     size_t nleft;
//     ssize_t nwritten;

//     nleft = n;
//     while ( nleft > 0 ) {
//         if ( ( nwritten = write( fd, ptr, nleft ) < 0 ) ) {
//             if ( nleft == n )
//                 return -1;
//             else
//                 break;
//         } else if ( nwritten == 0 ) {
//             break;
//         }

//         nleft -= nwritten;
//         ptr += nwritten;
//     }

//     return ( n - nleft );
// }