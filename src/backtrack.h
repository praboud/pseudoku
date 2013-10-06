#ifndef __BACKTRACK_H__
#define __BACKTRACK_H__

#include "cell.h"

int puzzle_backtrack(puzzle puz, int *cost);
int puzzle_solution_count(puzzle puz, int max_solutions, int *cost);

#endif
