#include <assert.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <unistd.h>

#include "const.h"
#include "handler.h"
#include "message.h"
#include "tor.h"
#include "utils.h"

using namespace std;


PacketSwitch::PacketSwitch ( int switchNum, int prev, int next, int ipLow, int ipHigh,
        string filename ) {
    assert( switchNum > 0 && switchNum <= MAX_NSW );
    assert( prev <= MAX_NSW && prev != 0 );
    assert( next <= MAX_NSW && next != 0 );
    assert( ipLow >= 0 && ipLow < ipHigh );
    assert( ipHigh <= MAXIP );

    this->switchNum = switchNum;
    this->prev = prev;
    this->next = next;
    this->ipLow = ipLow;
    this->ipHigh = ipHigh;
    this->filename = filename;

    this->ftable.push_back( Rule(0, MAXIP, this->ipLow, this->ipHigh, FORWARD, 3) );

    memset( &fifos, 0, sizeof(fifos) );
    memset( &pfds, 0, sizeof(pfds) );
}

PacketSwitch::~PacketSwitch() {

}

void PacketSwitch::initFIFO () {
    int count = 0;

    fifos[0][0] = STDIN_FILENO;
    fifos[0][1] = STDOUT_FILENO;
    setPfd( count, fifos[0][0] );
    count++;

    mkopen( 0, switchNum, fifos[1][0], fifos[1][1] );

    setPfd( count, fifos[1][0] );
    count++;

    if ( prev != -1 ) {
        mkopen( prev, switchNum, fifos[2][0], fifos[2][1] );
        setPfd( count, fifos[2][0] );
    } count++;

    if ( next != -1 ) {
        mkopen( next, switchNum, fifos[3][0], fifos[3][1] );
        setPfd( count, fifos[3][0] );
    } count++;
}

void PacketSwitch::setPfd ( int i, int rfd ) {
    pfds[i].fd = rfd;
    pfds[i].events = POLLIN;
    pfds[i].revents = 0;
}

void PacketSwitch::info() {
    cout << endl << "Forwarding table" << endl;

    int count = 0;

    for ( auto it = ftable.begin(); it != ftable.end(); it++ ) {
        cout << "[" << to_string(count) << "]" << *it;
    }

    cout << "Packet Stats:" << endl;

    cout << "\tReceived " << "ADMIT:" << to_string(admitCount) << 
    ", HELLO_ACK:" << to_string(hello_ackCount) << 
    ", ADD:" << to_string(addCount) << 
    ", RELAYIN:" << to_string(relayinCount) << endl;

    cout << "\tTransmitted: " << "HELLO:" << to_string(helloCount) << 
    ", ASK:" << to_string(askCount) << 
    ", RELAYOUT:" << to_string(relayoutCount) << endl << endl;
}

void * PacketSwitch::readFile () {
    string sline = "";
    
    char line[1024];
    vector<string> tokens;
    
    memset( &line, 0, sizeof(line) );

    FILE *f = fopen( filename.c_str(), "r");
    assert ( f != nullptr );

    while ( hello_ackCount == 0 ) { sleep(3); }

    while ( fgets( line, 1024, f ) ) {
        if ( strlen(line) == 0 || line[0] == '#' || line[0] == '\n' || line[0] == ' ' ) {
            sline = "";
            tokens.clear();
            memset( &line, 0, sizeof(line) );
            continue;
        }

        sline = string(line);
        sline = sline.substr( 0, sline.find("\n") );
        split( sline, " ", tokens );

        processLine(tokens);

        sline = "";
        tokens.clear();
        memset( &line, 0, sizeof(line) );
    }

    return NULL;
}

/**
 * @brief Process each individual line in ".dat" file.
 * 
 * @param tokens 
 */
void PacketSwitch::processLine ( vector<string> &tokens ) {
    int swNum = stoSwNum( tokens.front() );
    if ( swNum != switchNum ) return;

    if ( tokens.at(1) == "delay" ) {
        int usec = stoi( tokens.back() );
        delay(usec);
        return;
    }
    
    int srcIP = stoi( tokens.at(1) );
    int destIP = stoi( tokens.back() );

    forwarding( srcIP, destIP, 0 );
}

void * PacketSwitch::startPoll () {
    int in = 0;
    int len = 0;
    int rval = 0;

    char buf[MAXBUF];
    string prefix;

    Frame frame;
    PacketType packetType;

    cout << "start polling" << endl << endl;

    sendHELLO();

    while (1) {
        rval = poll( pfds, MAXPKFD, 0 );
        len = 0;
        prefix = "";

        memset( &buf, 0, sizeof(buf) );
        memset( &frame, 0, sizeof(frame) );
        packetType = UNKNOWN;

        checkPendingPacket();

        for ( in = 0; in < MAXPKFD; in++ ) {
            if ( pfds[in].revents & POLLIN ) {
                if ( in == 0 ) {
                    len = read( fifos[in][0], buf, MAXBUF );
                    
                    if ( strcmp( buf, "info\n" ) == 0 )
                        info();
                    else if ( strcmp( buf, "exit\n" ) == 0 ) {
                        info();
                        exit(0);
                    } else 
                        warning( "a2w22: %s: command not found\n", buf );
                } else {

                frame = rcvFrame(fifos[in][0]);
                packetType = frame.packetType;


                prefix = makePrefix( in - 1, switchNum,0 );
                printFrame( prefix.c_str(), &frame ); 

                switch(packetType) {
                    case ADD:
                        rcvADD(frame.packet.addPacket);
                        break;
                    case HELLO_ACK:
                        hello_ackCount++;
                        break;
                    case RELAY:
                        rcvRELAY(frame.packet.replayPacket);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
}

string PacketSwitch::makePrefix ( int src, int dest, int option ) {
    string strSrc = ( src == 0 ) ? "master" : ( "psw" + to_string(src) );
    string strDest = ( dest == 0 ) ? "master" : ( "psw" + to_string(dest) );
    string strOption = option ? "Transmitted" : "Received";
    string prefix = strOption + " (src = " + strSrc + ", dest = " + strDest + ")";

    return prefix;
}

void PacketSwitch::addRule (ADDPacket addPk) {
    Rule newRule = Rule( addPk.srcIP_lo, addPk.srcIP_hi, 
                            addPk.destIP_lo, addPk.destIP_hi, 
                            addPk.actionType, addPk.actionVal );
    
    for ( auto it = ftable.begin(); it != ftable.end(); it++ ) {
        if ( it->isEqual(newRule) )
            return;
    }

    ftable.push_back(newRule);
}

void PacketSwitch::checkPendingPacket () {
    if ( !pending.empty() ) {
        auto it = pending.begin();
            
        while (it != pending.end()) {
            if ( forwardPacket( it->srcIP, it->destIP, 0 ) )
                it = pending.erase(it);
            else 
                it++;
        }
    }
}

void PacketSwitch::delay (int usec) {
    cout << "** Entering a delay period of " << to_string(usec) << " mesc" << endl << endl;

    usleep(usec);

    cout << "** Delay period ended" << endl << endl;

    return;
}

/**
 * @brief forwarding method include additional step. This step is to send an ASK 
 * packet if a rule isn't in the forwarding table.
 * 
 * @param srcIP 
 * @param destIP 
 * @param relayIn 
 */
void PacketSwitch::forwarding ( int srcIP, int destIP, int relayIn ) {
    int forwarded = forwardPacket( srcIP, destIP, relayIn );

    if (forwarded) return;
    
    sendASK ( srcIP, destIP );
}

/**
 * @brief Forwarding a packet.
 * 
 * @param srcIP 
 * @param destIP 
 * @param relayIn 
 * @return int Return 1 if forwarding successfully. 0 means there's no such a rule 
 * that match the sending request
 */
int PacketSwitch::forwardPacket ( int srcIP, int destIP, int relayIn ) {

    for ( auto it = ftable.begin(); it != ftable.end(); it++ ) {
        if ( it->isMatch(srcIP, destIP) ) {
            
            if ( it->actionType != DROP && !it->isReach(destIP) )
                sendRELAY( it->actionVal, srcIP, destIP );
            
            if ( !relayIn ) {
                it->pkCount++;
                admitCount++;
            }
            
            return 1;
        }
    }

    return 0;
}

void PacketSwitch::sendASK ( int srcIP, int destIP ) {
    PendingPacket pendingPacket = { srcIP, destIP };
    pending.push_back(pendingPacket);

    for ( auto it = askHistory.begin(); it != askHistory.end(); it++ ) {
        if ( srcIP == it->srcIP && destIP == it->destIP )
            return;
    }

    Packet askPk = composeASK( srcIP, destIP );

    string prefix = makePrefix( switchNum, 0, 1 );

    sendFrame( prefix.c_str(), fifos[1][1], ASK, &askPk );
    askHistory.push_back(pendingPacket);

    askCount++;
}

void PacketSwitch::sendHELLO () {
    Packet helloPk = composeHELLO( switchNum, prev, next, ipLow, ipHigh );

    string prefix = makePrefix( switchNum, 0, 1 );

    sendFrame( prefix.c_str(), fifos[1][1], HELLO, &helloPk );

    helloCount++;
}

void PacketSwitch::sendRELAY ( int actionVal, int srcIP, int destIP ) {
    Packet relayPk;
    memset( &relayPk, 0, sizeof(relayPk) );

    relayPk = composeRELAY( srcIP, destIP );

    switch(actionVal) {
        case 1: { // port 1
            string prefix = makePrefix( switchNum, prev, 1 );
            sendFrame( prefix.c_str(), fifos[2][1], RELAY, &relayPk );
            break;
        }               
        case 2: { // port 2
            string prefix = makePrefix( switchNum, next, 1 );
            sendFrame( prefix.c_str(), fifos[3][1], RELAY, &relayPk );
            break;
        }
    }

    relayoutCount++;
}

void PacketSwitch::rcvADD (ADDPacket addPk) {
    addRule(addPk);
    addCount++;
}

void PacketSwitch::rcvRELAY (RELAYPacket relayPk) {
    forwarding( relayPk.srcIP, relayPk.destIP, 1 );
    relayinCount++;
}
