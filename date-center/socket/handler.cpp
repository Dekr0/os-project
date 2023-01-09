#include "handler.h"


void warning ( const char *fmt, ...) {
    va_list ap;
    
    fflush(stdout);
    
    va_start( ap, fmt );
    vfprintf( stderr, fmt, ap );
    va_end(ap);
}

void fatal (const char *fmt, ...) {
    va_list ap;

    fflush(stdout);

    va_start( ap, fmt );
    vfprintf( stderr, fmt, ap );
    va_end(ap);

    fflush(NULL);
}