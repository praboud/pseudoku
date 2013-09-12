#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DEBUG 0
#define dprintf(fmt, ...) do { if (DEBUG) { fprintf(stderr, fmt, ##__VA_ARGS__); } } while (0)
#define puzzle_dprint(puz) do { if (DEBUG) { puzzle_print(puz, stderr); } } while (0)
#define pencil_dprint(m) do { if (DEBUG) { pencil_print(m, stderr); } } while (0)

#endif
