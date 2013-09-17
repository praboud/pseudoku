#ifndef __PUZZLE_H__
#define __PUZZLE_H__

#include "cell.h"
int puzzle_read(puzzle puz, FILE *f);
void puzzle_pencil_possibilities(puzzle puz);
void puzzle_print(puzzle puz, FILE *f);
void puzzle_print_short(puzzle puz, FILE *f);
int puzzle_is_consistent(puzzle puz);
int puzzle_noninked_count(puzzle puz);
void puzzle_copy(puzzle src, puzzle dst);
void puzzle_init(puzzle puz);
void puzzle_fill_cell(puzzle puz, int x, int y, int n);
void puzzle_clear_cell(puzzle puz, int x, int y);

#endif
