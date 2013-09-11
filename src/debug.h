#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#define DEBUG 0
#define dprintf(fmt, ...) do { if (DEBUG) { fprintf(stderr, fmt, ##__VA_ARGS__); } } while (0)

#endif
