#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "cell.h"
#include "puzzle.h"
#include "strategy.h"
#include "backtrack.h"
#include "constants.h"

void _scramble(int *array, int const len) {
    if (len > 1) {
        for (int i = 0; i < len - 1; i++) {
            int j = i + rand() % (len - i);
            assert(j >= i && j < len);
            int t = array[i];
            array[i] = array[j];
            array[j] = t;
        }
    }
}

void _random_indices(int *array, int const start, int const end) {
    const int len = end - start;
    for (int i = 0; i < len; i++) {
        array[i] = i + start;
    }
    _scramble(array, len);
    /* for (int i = 0; i < len; i++) { */
    /*     printf("num: %i %d\n", i, array[i]); */
    /* } */
}

void _fill_puzzle(puzzle blank) {
    // TODO: this does not seem to always terminate
    puzzle copy;

    int indices[BOARD_LENGTH];
    int possibilities[INK_END];
    _random_indices(indices, 0, BOARD_LENGTH);

    /* clear board, allow all possibilities */
    puzzle_init(blank);
    /* fill each of the 81 cell in turn, filling with a random possibilty
     * after each fill, check that a solution still exists */
    for (int i = 0; i < BOARD_LENGTH; i++) {
        int x = indices[i] % GROUP_LENGTH;
        int y = indices[i] / GROUP_LENGTH;
        _random_indices(possibilities, INK_START, INK_END + 1);
        assert(!blank[x][y].complete);
        for (int n = 0; n <= INK_END - INK_START; n++) {
            int p = possibilities[n];
            if (blank[x][y].u.pencil & ink_to_pencil(p)) {
                puzzle_copy(blank, copy);
                puzzle_fill_cell(copy, x, y, p);
                /* check to make sure the puzzle is still solvable */
                if (puzzle_backtrack(copy)) {
                    /* then fill the actual copy of the puzzle with that number */
                    puzzle_fill_cell(blank, x, y, p);
                    break;
                }
            }
        }
        // TODO: this assertion seems like it should fail sometimes
        assert(blank[x][y].complete);
        assert(puzzle_noninked_count(blank) == BOARD_LENGTH - i - 1);
    }
}

void _remove_cells(puzzle puz, int max_remove) {
    int indices[BOARD_LENGTH];
    puzzle copy;
    _random_indices(indices, 0, BOARD_LENGTH);
    assert(max_remove <= BOARD_LENGTH);
    for (int i = 0; i < max_remove; i++) {
        int x = indices[i] % GROUP_LENGTH;
        int y = indices[i] / GROUP_LENGTH;
        /* clear cell on copy, then solve to find the number of solutions
         * possible. If there is still a unique solution, then wipe this
         * cell on the actual working copy of the puzzle */
        puzzle_copy(puz, copy);
        puzzle_clear_cell(copy, x, y);
        puzzle_pencil_possibilities(copy);
        int solution_count = puzzle_solution_count(copy, 2);
        assert(solution_count > 0);

        if (solution_count == 1) {
            puzzle_clear_cell(puz, x, y);
        }
    }
}

void puzzle_generate(puzzle puz) {
    srand(time(NULL));
    _fill_puzzle(puz);
    _remove_cells(puz, 81);
}
