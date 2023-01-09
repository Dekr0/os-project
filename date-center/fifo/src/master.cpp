#include <assert.h>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>

#include <unistd.h>

#include "const.h"
#include "handler.h"
#include "master.h"
#include "message.h"
#include "utils.h"

using namespace std;


MasterSwitch::MasterSwitch (int numSwitch) {
    assert( numSwitch > 0 && numSwitch <= MAX_NSW );

    this->numSwitch = numSwitch;

    memset( &fifos, 0, sizeof(fifos) );
    memset( &pfds, 0, sizeof(pfds) );
}

void MasterSwitch::initFIFO () {
    fifos[0][0] = STDIN_FILENO;
    fifos[0][1] = STDOUT_FILENO;
    setPfd( 0, STDIN_FILENO );

    for ( int i = 1; i <= numSwitch; i++ ) {
        mkopen( i, 0, fifos[i][0], fifos[i][1] );
        setPfd( i, fifos[i][0] );
    }
}

void MasterSwitch::info () {
    cout << endl << "Switch information:" << endl;

    for ( auto it = switchInfos.begin(); it != switchInfos.end(); it++ ) {
        cout << "[psw" << to_string(it->swNum) << "]" << 
        " port1 = " << to_string(it->prev) << 
        ", port2 = " << to_string(it->next) << 
        ", port3 = " << to_string(it->ipLow) << "-" << to_string(it->ipHigh) << endl;
    }

    cout << endl;

    cout << "Packet Stats:" << endl;
    
    cout << "\tReceived:\t" << "HELLO:" << to_string(helloCount) << 
    ", ASK:" << to_string(askCount) << endl;

    cout << "\tTransmitted: " << "HELLO_ACK:" << to_string(hello_ackCount) << 
    ", ADD:" << to_string(addCount) << endl << endl;
}

void MasterSwitch::setPfd ( int i, int rfd ) {
    pfds[i].fd = rfd;
    pfds[i].events = POLLIN;
    pfds[i].revents = 0;
}

void MasterSwitch::addSwitchInfo (HELLOPacket pk) {
    SwitchInfo switchInfo;

    memset( (char *)&switchInfo, 0, sizeof(switchInfo) );

    switchInfo.swNum = pk.switchNum;
    switchInfo.prev = pk.prev;
    switchInfo.next = pk.next;
    switchInfo.ipLow = pk.srcIP_lo;
    switchInfo.ipHigh = pk.srcIP_hi;

    auto pos = switchInfos.begin() + switchInfo.swNum - 1;

    switchInfos.insert( pos, switchInfo );
}

Packet MasterSwitch::generateRule ( int switchNum, int destIP ) {
    Packet rule;
    memset( &rule, 0, sizeof(rule) );

    for ( auto it = switchInfos.begin(); it != switchInfos.end(); it++ ) {
        if ( destIP >= it->ipLow && destIP <= it->ipHigh ) {
            int actionVal;

            if ( switchNum == it->swNum )
                actionVal = 3; // port 3
            else if ( switchNum > it->swNum )
                actionVal = 1; // port 2
            else
                actionVal = 2; // port 1

            rule = composeADD( 0, MAXIP, it->ipLow, it->ipHigh, FORWARD, actionVal, 0 );
            return rule;
        }
    }

    rule = composeADD( 0, MAXIP, destIP, destIP, DROP, 0, 0 );

    return rule;
}

void MasterSwitch::sendADD ( int switchNum, int destIP, int wfd ) {
    Packet addPk;
    memset( &addPk, 0, sizeof(addPk) );

    addPk = generateRule( switchNum, destIP );

    string prefix = "Transmitted (src = master, dest = " + to_string(switchNum) + ")";

    sendFrame( prefix.c_str(), wfd, ADD, &addPk );

    addCount++;
}

void MasterSwitch::sendHELLO_ACK ( int switchNum, int wfd ) {
    Packet hello_ackPk;

    memset( &hello_ackPk, 0, sizeof(hello_ackPk) );

    hello_ackPk = composeHELLO_ACK();

    string prefix = "Transmitted (src = master, dest = " + to_string(switchNum) + ")";

    sendFrame( prefix.c_str(), wfd, HELLO_ACK, &hello_ackPk );

    hello_ackCount++;
}

void MasterSwitch::startPoll () {
    int in = 0;
    int len = 0;
    int rval = 0;
    

    char buf[MAXBUF];
    string prefix;

    Frame frame;
    PacketType packetType;

    cout << "start polling" << endl << endl;

    while (1) {
        rval = poll( pfds, MAXMSFD, 0 );
        len = 0;
        prefix = "";

        memset( &buf, 0, sizeof(buf) );
        memset( &frame, 0, sizeof(frame) );
        packetType = UNKNOWN;

        for ( in = 0; in < MAXMSFD; in++ ) {
            if ( pfds[in].revents & POLLIN ) {
                
                if ( in == 0 ) {
                    len = read( fifos[in][0], buf, MAXBUF );

                    if ( strcmp( buf, "info\n" ) == 0 )
                        info();
                    else if ( strcmp( buf, "exit\n" ) == 0 ) {
                        info();
                        return;
                    } else 
                        warning( "a2w22: %s: command not found\n", buf );
                    
                    memset( &buf, 0, sizeof(buf) );

                } else {
                    
                    frame = rcvFrame( fifos[in][0] );
                    packetType = frame.packetType;
                    
                    prefix = "\nReceived (src = psw" + to_string(in) + ", dest = master)";
                    
                    printFrame( prefix.c_str(), &frame );

                    switch(packetType) {
                        case ASK:
                            askCount++;
                            sendADD( in, frame.packet.askPacket.destIP, fifos[in][1] );
                            break;
                        case HELLO:
                            helloCount++;
                            addSwitchInfo(frame.packet.helloPacket);
                            sendHELLO_ACK( in, fifos[in][1] );
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
}
