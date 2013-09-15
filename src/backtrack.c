#include "puzzle.h"
#include "iter.h"
#include <assert.h>
#include "debug.h"

struct step {
    uint8_t x;
    uint8_t y;
    uint16_t pencil;
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

int _fill_cell(puzzle puz, struct step **stack, int x, int y, int last_guess) {
    int next = _next_possibility(puz[x][y].u.pencil, last_guess);
    assert(next >= 0 && next <= 9);
    if (next == 0) {
        return 0;
    } else {
        struct step *s = *stack;
        s->x = x;
        s->y = y;
        s->pencil = puz[x][y].u.pencil;
        *stack = ++s;
        puz[x][y].complete = 1;
        puz[x][y].u.ink = next;
        dprintf("trying %d next\n", next);
        dprintf("last = %d, next = %d\n", last_guess, next);
        puzzle_dprint(puz);
        return 1;
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
    while (*x < 9 && *y < 9 && puz[*x][*y].complete) {
        (*x)++;
        *y += *x / 9;
        *x %= 9;
    }
    assert(d <= *y * 9 + *x);
    return *x < 9 && *y < 9;
}

int _run_backtrack(puzzle puz, struct step * const stack, struct step **stackpp, int *x, int *y, int last_tried) {
    struct step *stackp = *stackpp;
    if(!_fill_cell(puz, &stackp, *x, *y, last_tried)) {
        dprintf("initial fill failed\n");
        return 0;
    }
    while (1) {
        dprintf("s = %ld, x = %d, y = %d\n", stackp - stack, *x, *y);
        assert(puz[*x][*y].complete);
        if (_consistent(puz, *x, *y)) {
            dprintf("consistent\n");
            if (!_next_unfilled(puz, x, y)) {
                dprintf("done\n");
                *stackpp = stackp;
                return 1;
            } else {
                dprintf("progressing\n");
                dprintf("before: %ld", stackp - stack);
                if (!_fill_cell(puz, &stackp, *x, *y, 0)) {
                    dprintf("non-initial fill failed\n");
                    return 0;
                }
                dprintf(", after: %ld\n", stackp - stack);
            }
        } else {
            dprintf("starting to back up\n");
            stackp--;
            do {
                dprintf("backtracking, s = %ld, x = %d, y = %d\n", stackp - stack, *x, *y);
                assert(puz[*x][*y].complete);

                /* fill_cell increments stackp, so decrement to get last step
                 * written and prepare to overwrite the step*/
                if (stackp == stack) {
                    /* we have no where to turn back to, so fail out */
                    dprintf("ran out of options\n");
                    return 0;
                }
                assert(stackp >= stack);
                assert(stackp->x == *x && stackp->y == *y);
                uint8_t last_tried = puz[*x][*y].u.ink;
                puz[*x][*y].complete = 0;
                puz[*x][*y].u.pencil = stackp->pencil;
                struct step *temp = stackp;
                if (!_fill_cell(puz, &stackp, *x, *y, last_tried)) {
                    assert(temp == stackp);
                    stackp--;
                    /* we have exhausted options, so we must have guessed badly
                     * at some point before; therefore, we turn back
                     * back up even further, to 2 steps ago,
                     * change coordinates to set up for next round */
                    *x = stackp->x;
                    *y = stackp->y;
                    dprintf("backing up to x = %d, y = %d\n", *x, *y);
                    assert(*x >= 0 && *x < 9);
                    assert(*y >= 0 && *y < 9);
                    assert(puz[*x][*y].u.ink >= 1 && puz[*x][*y].u.ink <= 9);
                    assert(puz[*x][*y].complete);
                } else {
                    dprintf("success!\n");
                    break;
                }
            } while(1);
        }
    }
}

int puzzle_backtrack(puzzle puz) {
    int stack_size = puzzle_noninked_count(puz);
    struct step stack[stack_size];
    struct step *stackp = stack;
    int success;
    int x = 0;
    int y = 0;
    if (!_next_unfilled(puz, &x, &y)) {
        return 1;
    }
    if ((success = _run_backtrack(puz, stack, &stackp, &x, &y, 0))) {
        assert(puzzle_is_consistent(puz));
        assert(puzzle_noninked_count(puz) == 0);
    }
    return success;
}

int puzzle_is_unique(puzzle puz) {
    int stack_size = puzzle_noninked_count(puz);
    struct step stack[stack_size];
    struct step *stackp = stack;
    int solution_count = 0;
    int x = 0;
    int y = 0;
    int last_tried = 0;
    if (!_next_unfilled(puz, &x, &y)) {
        return 1;
    }
    while (_run_backtrack(puz, stack, &stackp, &x, &y, last_tried) && solution_count < 2) {
        assert(puzzle_is_consistent(puz));
        assert(puzzle_noninked_count(puz) == 0);
        solution_count++;
        assert(stackp - stack == stack_size);
        stackp--;
        x = stackp->x;
        y = stackp->y;
        assert(puz[x][y].complete);
        last_tried = puz[x][y].u.ink;
        puz[x][y].complete = 0;
        puz[x][y].u.pencil = stackp->pencil;
    }
    return solution_count;
}
