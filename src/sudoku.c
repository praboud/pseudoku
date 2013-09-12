#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "iter.h"
#include "debug.h"
#include "strategy.h"
#include "puzzle.h"
#include "backtrack.h"

/* forward definitions */
void puzzle_print(puzzle puz, FILE *f);

void puzzle_solve (puzzle puz) {
    int change = 1;
    while (change) {
        change = 0;
        for (int strat = 0; strat < strat_count; strat++) {
            change |= solve_strategies[strat](puz);
            dprintf("\n");
        }
    }
    assert(puzzle_is_consistent(puz));
}

int main (void) {
    /* uint16_t x; */
    /* scanf("%hd", &x); */
    /* printf("%x, %d\n", x, hamming_weight(x)); */
    puzzle puz;
    puzzle_read(puz, stdin);
    puzzle_pencil_possibilities(puz);
    puzzle_print(puz, stdout);
    putc('\n', stdout);

    puzzle_solve(puz);
    puzzle_print(puz, stdout);
    putc('\n', stdout);

    backtrack(puz);
    puzzle_print(puz, stdout);
}
