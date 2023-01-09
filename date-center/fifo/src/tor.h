#ifndef TOR_H
#define TOR_H


#include <queue>
#include <string>
#include <vector>

#include <poll.h>
#include <pthread.h>

#include "rule.h"

using namespace std;


typedef struct {
    int srcIP;
    int destIP;
} PendingPacket;


class PacketSwitch {
    public:
        int switchNum;
        int prev, next;
        int ipLow, ipHigh;

        string filename;

        int admitCount = 0, hello_ackCount = 0, addCount = 0, relayinCount = 0;
        int helloCount = 0, askCount = 0, relayoutCount = 0;

        PacketSwitch ( int switchNum, int prev, int next, int ipLow, int ipHigh,
        string filename );

        void initFIFO ();
        void * startPoll ();
        void info ();
        string makePrefix ( int src, int dest, int option ); 
        
        void * readFile ();
        void processLine ( vector<string> &tokens );
        
        void addRule (ADDPacket addPk);
        void delay (int usec);
        void forwarding ( int srcIP, int destIP, int relayIn );
        int forwardPacket ( int srcIP, int destIP, int relayIn );
        void checkPendingPacket ();
        void sendASK ( int srcIP, int destIP );
        void sendHELLO ();
        void sendRELAY ( int actionVal, int srcIP, int destIP );
        void rcvADD (ADDPacket addPk);
        void rcvRELAY (RELAYPacket relayPk);

        ~PacketSwitch();

    private:
        // 0 -> stdin, stdout
        // 1 -> port 0, master
        // 2 -> port 1, prev
        // 3 -> port 2, next
        int fifos[MAXPKFD][2];
        struct pollfd pfds[MAXPKFD];

        vector<PendingPacket> pending;
        vector<PendingPacket> askHistory;
        vector<Rule> ftable;

        void setPfd ( int i, int rfd );

};


#endif