#include <stdarg.h>

#ifndef HANDLER_H
#define HANDLER_H

/**
 * Functions WARNING and FATAL that can be used to report warnings and
 * errors after system calls. Direct clone version of code in lab 1 starter.c
 */

void warning ( const char *fmt, ... );

void fatal ( const char *fmt, ... );

#endif
