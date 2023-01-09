#ifndef RULE_H
#define RULE_H


#include <iostream>

using namespace std;


class Rule {
    public:
        int srcIP_lo, srcIP_hi;
        int destIP_lo, destIP_hi;
        int actionType, actionVal;
        int pkCount = 0;

        Rule (int srcIP_lo, int srcIP_hi, int destIP_lo, int destIP_hi, 
        int actionType, int actionVal );
        
        friend ostream& operator<< ( ostream& out, const Rule& r );

        bool isMatch( int srcIP, int destIP );
        bool isReach( int destIP );
        bool isEqual( const Rule& r );
};


#endif