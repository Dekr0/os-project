#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "const.h"
#include "handler.h"
#include "master.h"
#include "tor.h"
#include "utils.h"

using namespace std;


static int master = 0;

struct sigaction oldAct, newAct;

static MasterSwitch *masterSwitch;
static PacketSwitch *packetSwitch;

void infoHanlder ( int sigNo ) {
    if (master) 
        masterSwitch->info();
    else
        packetSwitch->info();
}

int main ( int argc, char *args[] ) {
    newAct.sa_handler = &infoHanlder;
    newAct.sa_flags |= SA_SIGINFO;

    sigaction( SIGUSR1, &newAct, &oldAct );

    if ( argc == 3 ) {
        int numSwitch = atoi(args[2]);

        if ( numSwitch > 0 && numSwitch <= MAX_NSW ) {
            master = 1;
            masterSwitch = new MasterSwitch(numSwitch);
            masterSwitch->initFIFO();
            masterSwitch->startPoll();

            delete masterSwitch;
        }
    } else if ( argc == 6 ) {

        int switchNum = stoSwNum( string(args[1]) );
        int prev = stoSwNum( string(args[3]) );
        int next = stoSwNum( string(args[4]) );

        vector<string> ipRange;

        split( string(args[5]), "-", ipRange );

        int ipLow = stoi(ipRange.front());
        int ipHigh = stoi(ipRange.back());

        string filename = string(args[2]);

        packetSwitch = new PacketSwitch( switchNum, prev, next, ipLow, ipHigh, 
            filename );

        packetSwitch->initFIFO();

        int rval;
        pthread_t fileThreadId, pollThreadId;

        rval = pthread_create( &fileThreadId, NULL, (THREADFUNCPTR)&PacketSwitch::readFile, packetSwitch );

        if (rval) {
            fatal( "pthread_create() error" );
            exit(1);
        }

        rval = pthread_create( &pollThreadId, NULL, (THREADFUNCPTR)&PacketSwitch::startPoll, packetSwitch );

        if (rval) {
            fatal( "pthread_create() error" );
            exit(1);
        }

        rval = pthread_join( fileThreadId, NULL );
        if (rval) {
            fatal( "pthread_join() error %d", rval );  
        }

        rval = pthread_join( pollThreadId, NULL );
        if (rval) {
            fatal( "pthread_join() error %d", rval );  
        }

        delete packetSwitch;
    }

    return 0;
}