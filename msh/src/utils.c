#include "handler.h"
#include "utils.h"


int gsub ( char *inStr, char *filter, char *repStr ) {
    int numMatch = 0;
    int matched;
    char outStr[MAXLEN]; // output string without omitted string
    memset( outStr, 0, sizeof(outStr) );

    for ( size_t i = 0; i < strlen(inStr); i++ ) {

        matched = 0;

        // Check whether if current character of the input string is in a set of
        // provided characters for matching
        for ( size_t j = 0; j < strlen(filter); j++ ) {
            if ( inStr[i] == filter[j] ) {
                // find a match, append it with the replacement string
                strcat( outStr, repStr );

                matched = 1;
                numMatch++;

                break;
            }
        }

        // No match, append it normally without modification
        if (matched == 0) {
            outStr[ strlen(outStr) ] = inStr[i];
        }
    }

    memset( inStr, 0, sizeof(inStr) );
    strcpy( inStr, outStr );  // copy the output string to input string

    return numMatch;
}

void printTime ( clock_t wallTime, struct tms *tmsStart, struct tms *tmsEnd ) {
    long clockTick = 0; // unit, clock ticks per second

    if ( clockTick == 0 ) {
        if ( (clockTick = sysconf(_SC_CLK_TCK)) < 0 ) {
            warning( "Failed to fetch clock ticks per second\n" );
        }
    }

    printf( "   real:   %7.2f\n", wallTime / (double) clockTick );
    
    printf( "   user:   %7.2f\n", 
        ( tmsEnd->tms_utime - tmsStart->tms_utime ) / (double) clockTick );
    
    printf( "   sys:   %7.2f\n", 
        ( tmsEnd->tms_stime - tmsStart->tms_stime ) / (double) clockTick );
    
    printf( "   child user:   %7.2f\n", 
        ( tmsEnd->tms_cutime - tmsStart->tms_cutime ) / (double) clockTick );

    printf( "   child sys:   %7.2f\n", 
        ( tmsEnd->tms_cstime - tmsStart->tms_cstime ) / (double) clockTick );

}

char *pathAlloc (size_t *sizePtr) {
    char *ptr = NULL; 
    int errno = 0;
    size_t size = 0;

    if (maxPath == 0) {
        errno = 0;

        if ( ( maxPath = pathconf( "/", _PC_PATH_MAX ) ) < 0 ) {
            // obtain the max buffer size of path from macro _PC_PATH_MAX
            if ( errno == 0 ) {
                // Guess the max buffer size of path if it cannot obtain the macro
                maxPath = GUESSPATHLEN;
            } else {
                warning( "pathconf error for _PC_PATH_MAX\n" );
            }
        } else {
            maxPath++; // extra byte for null character
        }
    }

    size = maxPath;
    ptr = malloc(size);

    if ( sizePtr != NULL ) {
        *sizePtr = size;
    }

    return ptr;
}

int startWith (const char *str, const char compare) {
    if ( str[0] == compare )
        return 1;

    return 0;
}

int split ( char *inStr, char tokens[][MAXCHARS], char fs[] ) {
    int numToken = 0;

    char *token = NULL;

    token = strtok( inStr, fs );
    while ( token != NULL ) {
        strcpy(tokens[numToken++], token);

        // continue to consume the previous input buffer to split 
        token = strtok( NULL, fs ); 
    }

    token = NULL;

    return numToken;
}
