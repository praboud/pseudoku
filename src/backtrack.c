#include "backtrack.h"
#include "puzzle.h"
#include "iter.h"
#include <assert.h>
#include "debug.h"
#include "strategy.h"
#include "constants.h"

#define COST_GUESS 10000

struct step {
    uint8_t x;
    uint8_t y;
    puzzle p;
    int cost;
};

int _next_possibility(uint16_t pencil, int last) {
    int next = pencil >> last;
    if (next) {
        next = __builtin_ffs(next) + last;
        assert(next > last);
    } else {
        next = 0;
    }
    return next;
}

int _fill_cell(puzzle puz, struct step **stack, int x, int y, int last_guess, int *cost) {
    int next = _next_possibility(puz[x][y].u.pencil, last_guess);
    assert(next >= 0 && next <= 9);
    if (next == 0) {
        return 0;
    } else {
        struct step *s = *stack;
        assert(0 <= x && x < 9);
        assert(0 <= y && y < 9);
        s->x = x;
        s->y = y;

        /* save cost before this additional guess, then increment cost to account
         * for increased guess level (n guesses "deep")*/
        s->cost = *cost;
        *cost += COST_GUESS;
        puzzle_copy(puz, s->p);
        (*stack)++;
        puzzle_fill_cell(puz, x, y, next);
        dprintf("trying %d next\n", next);
        dprintf("last = %d, next = %d\n", last_guess, next);
        puzzle_dprint(puz);
        return next;
    }
}

int _consistent(puzzle puz, int x, int y) {
    struct iter it;
    dprintf("row %d\n", y);
    iter_init(&it, ROW, y);
    if (!iter_consistent(&it, puz)) return 0;
    dprintf("col %d\n", x);
    iter_init(&it, COL, x);
    if (!iter_consistent(&it, puz)) return 0;
    dprintf("box\n");
    iter_init(&it, BOX, (y / 3) * 3 + x / 3);
    if (!iter_consistent(&it, puz)) return 0;
    return 1;
}

int _next_unfilled(puzzle puz, int *x, int *y) {
    int d = *y * 9 + *x;
    UNUSED(d);
    while (*x < 9 && *y < 9 && puz[*x][*y].complete) {
        (*x)++;
        *y += *x / 9;
        *x %= 9;
    }
    assert(d <= *y * 9 + *x);
    return *x < 9 && *y < 9;
}

int _run_backtrack(puzzle puz, struct step * const stack, struct step **stackpp, int *x, int *y,
                   int last_tried, int *cost) {
    struct step *stackp = *stackpp;
    assert(last_tried >= 0 && last_tried <= 9);
    while (1) {
        dprintf("s = %ld, x = %d, y = %d\n", stackp - stack, *x, *y);
        if (puzzle_logic(puz, cost) != INCONSISTENT) {
            dprintf("consistent\n");
            if (!_next_unfilled(puz, x, y)) {
                dprintf("done\n");
                *stackpp = stackp;
                return 1;
            } else {
                assert(!puz[*x][*y].complete);
                dprintf("progressing\n");
                dprintf("before: %ld", stackp - stack);
                if (!(last_tried = _fill_cell(puz, &stackp, *x, *y, 0, cost))) {
                    dprintf("non-initial fill failed\n");
                    return 0;
                }
                assert(last_tried >= 1 && last_tried <= 9);
                dprintf(", after: %ld\n", stackp - stack);
            }
        } else {
            dprintf("starting to back up\n");
            stackp--;
            do {
                dprintf("backtracking, s = %ld, x = %d, y = %d\n", stackp - stack, *x, *y);

                /* fill_cell increments stackp, so decrement to get last step
                 * written and prepare to overwrite the step*/
                if (stackp < stack) {
                    /* we have no where to turn back to, so fail out */
                    dprintf("ran out of options\n");
                    return 0;
                }
                assert(stackp >= stack);
                assert(stackp->x == *x && stackp->y == *y);
                *cost = stackp->cost;
                last_tried = puz[*x][*y].u.ink;
                puzzle_copy(stackp->p, puz);
                puz[*x][*y].complete = 0;
                puz[*x][*y].u.pencil = stackp->p[*x][*y].u.pencil;
                assert(last_tried >= 0 && last_tried <= 9);
                if (last_tried >= 9 || !(last_tried = _fill_cell(puz, &stackp, *x, *y, last_tried, cost))) {
                    stackp--;
                    if (stackp < stack) {
                        return 0;
                    }
                    /* we have exhausted options, so we must have guessed badly
                     * at some point before; therefore, we turn back
                     * back up even further, to 2 steps ago,
                     * change coordinates to set up for next round */
                    *x = stackp->x;
                    *y = stackp->y;
                    *cost = stackp->cost;
                    dprintf("backing up to x = %d, y = %d\n", *x, *y);
                    assert(*x >= 0 && *x < 9);
                    assert(*y >= 0 && *y < 9);
                } else {
                    assert(last_tried >= 1 && last_tried <= 9);
                    dprintf("success!\n");
                    break;
                }
            } while(1);
        }
    }
}

int puzzle_backtrack(puzzle puz, int *cost) {
    return puzzle_solution_count(puz, 1, cost);
}

int puzzle_solution_count(puzzle puz, int max, int *cost) {
    /* note that the result taken from the cost parameter is meaningless, if max != 1 */
    int stack_size = puzzle_noninked_count(puz);
    struct step stack[stack_size];
    struct step *stackp = stack;
    int solution_count = 0;
    int x = 0;
    int y = 0;
    int last_tried = 0;
    int cost_dummy = 0;
    if (cost == NULL) {
        cost = &cost_dummy;
    }
    while (_run_backtrack(puz, stack, &stackp, &x, &y, last_tried, cost) && solution_count < max) {
        solution_count++;
        assert(puzzle_is_consistent(puz));
        assert(puzzle_noninked_count(puz) == 0);
        if (stackp <= stack) {
            break;
        }
        stackp--;
        x = stackp->x;
        y = stackp->y;
        assert(puz[x][y].complete);
        last_tried = puz[x][y].u.ink;
        puzzle_copy(stackp->p, puz);
    }
    return solution_count;
}
