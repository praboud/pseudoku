#include <string.h>
#include <assert.h>

#include "strategy.h"
#include "cell.h"
#include "debug.h"
#include "puzzle.h"
#include "iter.h"
#include "constants.h"

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

int _puzzle_singleton_cell(puzzle puz) {
    int change = 0;
    dprintf("running singleton cell\n");
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            struct cell *c = &puz[i][j];
            if (!c->complete) {
                if (hamming_weight(c->u.pencil) == 1) {
                    /* then only one number can occupy this cell,
                     * so we can fill it in*/
                    puzzle_fill_cell(puz, i, j, pencil_to_ink(c->u.pencil));
                    change = 1;
                } else if (c->u.pencil == 0) {
                    /* then no number can occupy this cell,
                     * so the puzzle is inconsistent */
                    return INCONSISTENT;
                }
            }
        }
    }
    return change;
}

int _puzzle_singleton_number(puzzle puz) {
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
            uint16_t all_seen = 0;
            /* get union of all cells in group but the ith */
            while ((c = iter_next(&it, puz))) {
                uint16_t m = cell_coerce_pencil(c);
                all_seen |= m;
                for (int k = 0; k < j; k++) rst[k] |= m;
                for (int k = j + 1; k < 9; k++) rst[k] |= m;
                j++;
            }
            if (all_seen != ALL_POS) {
                /* then there is at least one number is not filled in,
                 * and cannot go in any of the remaining places.
                 * hence, the puzzle is inconsistent */
                return INCONSISTENT;
            }
            j = 0;
            iter_init(&it, t, i);
            struct coord co;
            while ((c = iter_next_c(&it, puz, &co))) {
                if (!c->complete) {
                    int x = c->u.pencil & ~rst[j];
                    dprintf("%s %d, pos = %d, pencil = %x, x = %d\n", iter_type_to_string[t], i, j, c->u.pencil, x);
                    int h = hamming_weight(x);
                    if (h == 1) {
                        puzzle_fill_cell(puz, co.x, co.y, pencil_to_ink(x));
                        change = 1;
                    } else if (h > 1) {
                        /* then there are two or more numbers which must
                         * occupy the same cell. this is impossible, hence
                         * the puzzle is inconsistent */
                        return INCONSISTENT;
                    }
                }
                j++;
            }
        }
    }
    return change;
}

int _puzzle_subgroup_exclusion(puzzle puz, struct iter *group,
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
            pencil_dprint(m);
            dprintf("clearing %s %d, skip at %d\n",
                    iter_type_to_string[cross_type], cross_num+c, cross_skip_start);
            iter_init_skip3(&it, cross_type, cross_num + c, cross_skip_start);
            int res = iter_mask(&it, puz, m);
            if (res == INCONSISTENT) {
                return INCONSISTENT;
            }
            change |= res;
        }
    }
    return change;
}

int _puzzle_subgroup_exclusion_all(puzzle puz) {
    struct iter it;
    int change = 0;
    dprintf("running subgroup exclusion\n");
    for (enum iter_type t = ROW; t <= BOXT; t++) {
        for (int i = 0; i < 9; i++) {
            dprintf("running subgroup exclusion on %s %d\n",
                    iter_type_to_string[t], i);

            iter_init(&it, t, i);
            change |= _puzzle_subgroup_exclusion(puz, &it, (t + 2) % 4, (i / 3) * 3,
                                                (i % 3) * 3);
        }
    }
    return change;
}

int _find_subsets(uint16_t *poss, struct cell **group,
                  const int SUBSET_SIZE_MAX,
                  int (*cb)(int len, int *indices,
                            struct cell **cs, uint16_t poss_union)) {
    UNUSED(poss);
    UNUSED(cb);
    if (SUBSET_SIZE_MAX <= 0) return 0;
    int change = 0;
    const int SUBSET_SIZE_MIN = 2;
    int subset[9];
    int subset_len = 0;
    int current = 0;
    while (1) {
        /* decide whether or not we report the subset */
        if (subset_len >= SUBSET_SIZE_MIN) {
            uint16_t poss_union = 0;
            for (int i = 0; i < subset_len; i++) {
                assert(poss[subset[i]]);
                poss_union |= poss[subset[i]];
            }
            if (hamming_weight(poss_union) == subset_len) {
                change |= cb(subset_len, subset, group, poss_union);
            }
        }
        /* get next viable number to try, skip poss equal to 0
         * for naked set, this is a complete cell,
         * for hidden set, this is a number that can only go in one place */
        while (current < 9 && !poss[current]) current++;

        /* justification: this works because for each partial subset,
         * we will augment it with every subset of the elements larger
         * than its largest */
        if (subset_len < SUBSET_SIZE_MAX && current < 9) {
            /* continue extending subset until too long,
             * the number in the subset is too great*/
            subset[subset_len++] = current++;
        } else {
            /* otherwise, turn back, and eliminate last numbers
             * from subset */
            while (subset_len > 0) {
                current = subset[--subset_len];
                do {
                    current++;
                } while (current < 9 && !poss[current]);
                if (current < 9) break;
            }
            if (subset_len == 0 && subset[0] >= 8) return change;
            subset[subset_len++] = current++;
        }
    }
    return change;
}

int _naked_cb(int len, int *set, struct cell **group, uint16_t poss_union) {
    int j = 0;
    int change = 0;
    dprintf("found naked set: ");
    pencil_dprint(poss_union);
    dprintf("\n");
    /* putchar('['); */
    /* for (int i = 0; i < len - 1; i++) { */
    /*     printf("%d, ", set[i]); */
    /* } */
    /* printf("%d]\n", set[len - 1]); */

    /* reverse union of possibilties to mask out cell possibilties */
    uint16_t mask = ~poss_union;

    /* for all incomplete cells in the group not part of the subset,
     * mask away the possibilities in the subset */
    for (int i = 0; i < 9; i++) {
        if (j < len && i == set[j]) {
            j++;
        } else if (!group[i]->complete) {
            dprintf("reducing on %d\n", i);
            change = change || (poss_union & group[i]->u.ink);
            group[i]->u.ink &= mask;
        }
    }
    return change;
}

int _puzzle_naked_set(puzzle puz) {
    int change = 0;
    struct cell *group[9];
    uint16_t possibilities[9];
    struct iter it;
    for (enum iter_type t = ROW; t <= BOX; t++) {
        for (int i = 0; i < 9; i++) {
            iter_init(&it, t, i);
            int nonzero_count = 0;
            puzzle_dprint(puz);
            dprintf("running naked set analysis on %s %d\n",
                    iter_type_to_string[t], i);

            for (int j = 0; j < 9; j++) {
                struct cell *c = iter_next(&it, puz);
                group[j] = c;
                if (c->complete) {
                    possibilities[j] = 0;
                } else {
                    possibilities[j] = c->u.pencil;
                    nonzero_count++;
                }
                possibilities[j] = c->complete ? 0 : c->u.pencil;
            }
            change |= _find_subsets(possibilities, group,
                                    nonzero_count, _naked_cb);
            dprintf("...done\n");
            puzzle_dprint(puz);
        }
    }
    return change;
}

int (*_strategies[])(puzzle) = {
    _puzzle_singleton_cell,
    _puzzle_singleton_number,
    _puzzle_subgroup_exclusion_all,
    _puzzle_naked_set
};

int _strategy_count = sizeof _strategies / sizeof _strategies[0];

int puzzle_solve (puzzle puz) {
    int change = 1;
    while (change) {
        change = 0;
        for (int strat = 0; strat < _strategy_count; strat++) {
            int res = _strategies[strat](puz);
            if (res == INCONSISTENT) {
                return INCONSISTENT;
            }
            change |= res;
            dprintf("\n");
        }
    }
    assert(puzzle_is_consistent(puz));
    return SOLVED;
}

