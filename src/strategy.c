#include <string.h>
#include <assert.h>

#include "strategy.h"
#include "debug.h"
#include "puzzle.h"
#include "iter.h"

/* solving strategies */
/* strategy functions must all take a puzzle, and return an integer
 * the integer is 1 if some change has been made, and 0 otherwise
 * the strategy applies some algorithm to either eliminate possibilities
 * from a cell, or conclusively determine some cell.
 * the calling function is otherwise agnostic to what the strategy does.
 * to be used by the solving function, it must be included in the
 * solve_strategies array. puzzle_solve calls these functions in
 * rotation until none of the functions can progress any further
 * (ie: * return 1)
 */

int puzzle_singleton_cell(puzzle puz) {
    int change = 0;
    dprintf("running singleton cell\n");
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            struct cell *c = &puz[i][j];
            if (!c->complete && hamming_weight(c->u.pencil) == 1) {
                int x = __builtin_ctz(c->u.pencil);
                c->complete = 1;
                c->u.ink = x + 1;
                change = 1;
            }
        }
    }
    if (change) puzzle_pencil_possibilities(puz);
    return change;
}

int puzzle_singleton_number(puzzle puz) {
    int change = 0;
    dprintf("running singleton number\n");
    for (enum iter_type t = ROW; t <= BOX; t++) {
        for (int i = 0; i < 9; i++) {
            struct iter it;
            struct cell *c;
            uint16_t rst[9];
            memset(rst, 0, sizeof rst);
            iter_init(&it, t, i);
            int j = 0;
            /* get union of all cells in group but the ith */
            while ((c = iter_next(&it, puz))) {
                uint16_t m = cell_coerce_pencil(c);
                for (int k = 0; k < j; k++) rst[k] |= m;
                for (int k = j + 1; k < 9; k++) rst[k] |= m;
                j++;
            }
            j = 0;
            iter_init(&it, t, i);
            while ((c = iter_next(&it, puz))) {
                if (!c->complete) {
                    int x = c->u.pencil & ~rst[j];
                    dprintf("%s %d, pos = %d, pencil = %x, x = %d\n", iter_type_to_string[t], i, j, c->u.pencil, x);
                    int h = hamming_weight(x);
                    assert(h == 0 || h == 1);
                    if (h == 1) {
                        c->complete = 1;
                        c->u.ink = __builtin_ctz(x) + 1;
                        change = 1;
                    }
                }
                j++;
            }
        }
    }
    if (change) puzzle_pencil_possibilities(puz);
    return change;
}

int puzzle_subgroup_exclusion(puzzle puz, struct iter *group,
                              enum iter_type cross_type, int cross_num,
                              int cross_skip_start) {
    uint16_t sub[3];
    memset(sub, 0, sizeof sub);
    struct cell *c;
    int i = 0;
    int change = 0;
    dprintf("crossing %s %d, skip = %d\n", iter_type_to_string[cross_type],
            cross_num, cross_skip_start);
    while ((c = iter_next(group, puz))) {
        if (!c->complete) {
            sub[i / 3] |= c->u.pencil;
        }
        i++;
    }
    /* lit bits belong to numbers which occur in exactly 1 subgroup */
    uint16_t candidates = sub[0] ^ sub[1] ^ sub[2] ^ (sub[0] & sub[1] & sub[2]);
    dprintf("groups are %x, %x, %x; candidates are %x\n",
            sub[0], sub[1], sub[2], candidates);
    if (candidates == 0) {
        return 0;
    }

    for (int c = 0; c < 3; c++) {
        uint16_t m = sub[c] & candidates;
        if (m) {
            struct iter it;
            #ifdef DEBUF
            pencil_print(m, stderr);
            #endif
            dprintf("clearing %s %d, skip at %d\n",
                    iter_type_to_string[cross_type], cross_num+c, cross_skip_start);
            iter_init_skip3(&it, cross_type, cross_num + c, cross_skip_start);
            change |= iter_mask(&it, puz, m);
        }
    }
    return change;
}

int puzzle_subgroup_exclusion_all(puzzle puz) {
    struct iter it;
    int change = 0;
    dprintf("running subgroup exclusion\n");
    for (enum iter_type t = ROW; t <= BOXT; t++) {
        for (int i = 0; i < 9; i++) {
            dprintf("running subgroup exclusion on %s %d\n",
                    iter_type_to_string[t], i);

            iter_init(&it, t, i);
            change |= puzzle_subgroup_exclusion(puz, &it, (t + 2) % 4, (i / 3) * 3,
                                                (i % 3) * 3);
        }
    }
    return change;
}

int (*solve_strategies[])(puzzle) = {
    puzzle_singleton_cell,
    puzzle_singleton_number,
    puzzle_subgroup_exclusion_all
};

int strat_count = sizeof solve_strategies / sizeof solve_strategies[0];