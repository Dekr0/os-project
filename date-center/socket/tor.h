#ifndef TOR_H
#define TOR_H


#include <cstring>
#include <cstdio>
#include <iostream>
#include <string>
#include <queue>
#include <vector>

#include <assert.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "message.h"
#include "rule.h"


#define STD 0
#define MASTER 1
#define PREV 2
#define NEXT 3
#define READEND 0
#define WRITEEND 1

using namespace std;


typedef struct {
    int srcIP;
    int destIP;
} PendingPacket;

// Note: handle when master disconnect

class PacketSwitch {
    public:
        int pkFd;
        int portNumber;
        int switchNum;
        int prev, next;
        int ipLow, ipHigh;

        string filename;
        string serverName;

        int admitCount = 0, hello_ackCount = 0, addCount = 0, relayinCount = 0;
        int helloCount = 0, askCount = 0, relayoutCount = 0;

        PacketSwitch ( int switchNum, int prev, int next, int ipLow, int ipHigh,
        string filename, string serverName, int portNumber );

        
        void connectMaster (); 
        int isMasterOnline ();
        
        void * readFile ();
        void processLine ( vector<string> &tokens );

        void * startPoll ();
        void info ();
        
        void addRule (ADDPacket addPk);
        void delay (int usec);
        void forwarding ( int srcIP, int destIP, int relayIn );
        int forwardPacket ( int srcIP, int destIP, int relayIn );
        void checkPendingPacket ();

        void sendASK ( int srcIP, int destIP );
        void sendDISCONNECT ();
        void sendHELLO ();
        void sendRELAY ( int actionVal, int srcIP, int destIP );
        
        void rcvADD (ADDPacket addPk);
        void rcvDISCONNECT (DISCONNECTPacket disconnectPacket);
        void rcvRELAY (RELAYPacket relayPk);

        void createFIFO ();
        string makePrefix ( int src, int dest, int option ); 

        ~PacketSwitch();

    private:
        // 0 -> stdin, stdout (Non fifos)
        // 1 -> port 0, master (Non fifos, socket)
        // 2 -> port 1, prev
        // 3 -> port 2, next
        int fds[MAXPKFD][2];
        struct pollfd pfds[MAXPKFD];

        vector<PendingPacket> pending;
        vector<PendingPacket> askHistory;
        vector<Rule> ftable;

        void setPfd ( int i, int rfd );
        void resetPfd (int i);

};


#endif