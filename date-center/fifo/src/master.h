#ifndef MASTER_H
#define MASTER_H


#include <vector>

#include <poll.h>

#include "message.h"

using namespace std;


typedef struct {
    int swNum;
    int prev, next;
    int ipLow, ipHigh;
} SwitchInfo;

class MasterSwitch {

    public:
        MasterSwitch (int numSwitch);
        void initFIFO ();
        void info ();
        
        void startPoll ();

        
        void sendADD ( int switchNum, int dest, int wfd );
        void sendHELLO_ACK ( int switchNum, int wfd );
        
        void addSwitchInfo (HELLOPacket helloPk);
        Packet generateRule ( int switchNum, int destIP );

    private:
        int numSwitch;

        int helloCount = 0, askCount = 0;
        int hello_ackCount = 0, addCount = 0;

        // 0 -> stdin, stdout
        // 1 ~ MAXNSW all packets switch
        int fifos[MAXMSFD][2];
        struct pollfd pfds[MAXMSFD];
        
        vector<SwitchInfo> switchInfos;
        
        int forkSwitches();
        
        void setPfd ( int i, int rfd );
        
};



#endif