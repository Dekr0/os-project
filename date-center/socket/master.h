#ifndef MASTER_H
#define MASTER_H


#include <assert.h>
#include <cstring>
#include <string>
#include <iostream>
#include <unordered_map>

#include <netdb.h>
#include <poll.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "master.h"
#include "message.h"

#define STDININDEX 0
#define MSWSFDINDEX 1


using namespace std;


typedef struct {
    int swNum;
    int prev, next;
    int ipLow, ipHigh;
} SwitchInfo;

class MasterSwitch {

    public:
        MasterSwitch ( int numSwitch, int portNumber );


        void createSocket();
        void bindSocket();
        void masterListen();
        void masterAccept();
        void masterDisconnect();
        
        void startPoll ();
        void info ();

        void sendADD ( int switchNum, int dest, int wfd );
        void sendHELLO_ACK ( int switchNum, int wfd );
        void sendDISCONNECT ( int switchNum, int wfd );
        
        void addSwitchInfo ( HELLOPacket helloPk, int pfdIndex );
        void lostConnection ( int pfdIndex );
        void removeSwitch ( DISCONNECTPacket disconnectPk, int pfdIndex );
        void removeSwitchInfo (int pfdIndex);
        Packet generateRule ( int switchNum, int destIP );

    private:
        // number of accepted connection (packet switch), 2 = 0 accepted connection
        int accpectedConn = 0;
        int numSwitch;
        int portNumber;
        int serverFd = -1;

        int helloCount = 0, askCount = 0;
        int hello_ackCount = 0, addCount = 0;

        // 0 -> stdin, stdout
        // 1 -> server socket
        // 2 ~ MAXMSFD -> packet switch connection
        struct pollfd pfds[MAXMSFD];
        
        // int -> position of pollfd in pfds
        // SwitchInfo -> information of packet switch "i"
        // one packet switch tied to one slot in pfds
        unordered_map<int, SwitchInfo> switchInfos;
        
        int forkSwitches();
        
        void resetPfd (int i);
        void setPfd ( int i, int rfd );

        
};



#endif