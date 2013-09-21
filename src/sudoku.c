#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "debug.h"
#include "strategy.h"
#include "puzzle.h"
#include "backtrack.h"
#include "constants.h"
#include "generator.h"
#include "interactive.h"

/* forward definitions */
void puzzle_print(puzzle puz, FILE *f);

void read_and_solve (void) {
    puzzle puz;
    puzzle_read(puz, stdin);
    puzzle_pencil_possibilities(puz);
    puzzle_print(puz, stdout);
    putc('\n', stdout);

    if (!puzzle_backtrack(puz)) {
        printf("The puzzle is inconsistent\n");
    } else {
        puzzle_print(puz, stdout);
    }
}

void generate(void) {
    puzzle puz;
    puzzle_generate(puz);
    puzzle_pencil_possibilities(puz);
    puzzle_print(puz, stdout);
}

void test_unique(void) {
    puzzle puz;
    puzzle_read(puz, stdin);
    puzzle_pencil_possibilities(puz);
    printf("%d solutions found\n", puzzle_solution_count(puz, 2));
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        char *command = argv[1];
        if (strcmp(command, "solve") == 0) {
            read_and_solve();
            return 0;
        } else if (strcmp(command, "generate") == 0) {
            generate();
            return 0;
        } else if (strcmp(command, "interactive") == 0) {
            interactive();
            return 0;
        } else if (strcmp(command, "unique") == 0) {
            test_unique();
            return 0;
        }
    }
    puts("Usage: ./sudoku [solve|generate|interactive|unique]");
    return 1;
}
