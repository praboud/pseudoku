#ifndef __DEBUG_H__
#define __DEBUG_H__

/* constants */
#define DEBUG 0
#define PIPE stdout

/* debug printing which can be easily switched on and off
 *
 * these macros use some weird constructs, taken from stack overflow
 *
 * do while -> so the compiler knows this is a statement, not an expression
 * if instead of #ifndef -> so the function call is always visible to the
 * compiler (so errors are detected), and the compiler will optimize the
 * if statement away anyway*/
#define dprintf(...) do { if (DEBUG) { fprintf(PIPE, ##__VA_ARGS__); } } while (0)
#define puzzle_dprint(puz) do { if (DEBUG) { puzzle_print(puz, PIPE); } } while (0)
#define pencil_dprint(m) do { if (DEBUG) { pencil_print(m, PIPE); } } while (0)

#define UNUSED(x) (void) (x)

#endif
