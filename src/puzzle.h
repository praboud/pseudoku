#ifndef __PUZZLE_H__
#define __PUZZLE_H__

#include "cell.h"
int puzzle_read(puzzle puz, FILE *f);
void puzzle_pencil_possibilities(puzzle puz);
void puzzle_print(puzzle puz, FILE *f);
int puzzle_is_consistent(puzzle puz);

#endif
