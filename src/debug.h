#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DEBUG 0
#define PIPE stdout
#define dprintf(...) do { if (DEBUG) { fprintf(PIPE, ##__VA_ARGS__); } } while (0)
#define puzzle_dprint(puz) do { if (0) { puzzle_print(puz, PIPE); } } while (0)
#define pencil_dprint(m) do { if (DEBUG) { pencil_print(m, PIPE); } } while (0)

#endif
