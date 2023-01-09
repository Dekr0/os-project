#include <string>

#include <assert.h>

#include "const.h"
#include "rule.h"

using namespace std;


Rule::Rule ( int srcIP_lo, int srcIP_hi, int destIP_lo, int destIP_hi, 
    int actionType, int actionVal ) {
    
    assert( srcIP_lo <= srcIP_hi && srcIP_lo >= 0 );
    assert( srcIP_hi <= MAXIP );
    assert( destIP_lo <= destIP_hi && destIP_lo >= 0 );
    assert( destIP_hi <= MAXIP );

    this->srcIP_lo = srcIP_lo;
    this->srcIP_hi = srcIP_hi;
    this->destIP_lo = destIP_lo;
    this->destIP_hi = destIP_hi;
    this->actionType = actionType;
    this->actionVal = actionVal;
}

bool Rule::isEqual ( const Rule& r ) {
    return srcIP_lo == r.srcIP_lo && 
           srcIP_hi == r.srcIP_hi && 
           destIP_lo == r.destIP_lo && 
           destIP_hi == r.destIP_hi && 
           actionType == r.actionType && 
           actionVal == r.actionVal;
}

bool Rule::isMatch ( int srcIP, int destIP ) {
    return ( srcIP >= srcIP_lo ) && ( srcIP <= srcIP_hi ) && ( destIP >= destIP_lo ) && ( destIP <= destIP_hi ) ;
}

bool Rule::isReach (int destIP) {
    return actionVal == 3 && destIP >= destIP_lo && destIP <= destIP_hi;
}

ostream& operator<< ( ostream& out, const Rule& r ) {
    string line = 
        "(srcIP = " + to_string(r.srcIP_lo) + "-" + to_string(r.srcIP_hi) + 
        ", destIP = " + to_string(r.destIP_lo) + "-" + to_string(r.destIP_hi) +
        ", action = " + actionNames[r.actionType] + ":" + to_string(r.actionVal) + 
        ", pkCount = " + to_string(r.pkCount) + ")";
    
    out << line << endl;

    return out;
}