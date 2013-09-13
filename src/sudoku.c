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
#include "constants.h"

/* forward definitions */
void puzzle_print(puzzle puz, FILE *f);

int main (void) {
    /* uint16_t x; */
    /* scanf("%hd", &x); */
    /* printf("%x, %d\n", x, hamming_weight(x)); */
    puzzle puz;
    puzzle_read(puz, stdin);
    puzzle_pencil_possibilities(puz);
    puzzle_print(puz, stdout);
    putc('\n', stdout);

    if (puzzle_solve(puz) == INCONSISTENT) {
        puzzle_print(puz, stdout);
        printf("The puzzle is inconsistent\n");
    } else {
        puzzle_print(puz, stdout);
        putc('\n', stdout);

        if (backtrack(puz) == 0) {
            puzzle_print(puz, stdout);
            printf("The puzzle is inconsistent\n");
        } else {
            puzzle_print(puz, stdout);
        }
    }
}
