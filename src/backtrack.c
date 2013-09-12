#include "puzzle.h"
#include "iter.h"
#include <assert.h>
#include "debug.h"

struct step {
    uint8_t x;
    uint8_t y;
    uint16_t pencil;
};

int next_possibility(uint16_t pencil, int last) {
    int next = pencil >> last;
    if (next) {
        next = __builtin_ffs(next) + last;
        assert(next > last);
    } else {
        next = 0;
    }
    return next;
}

int fill_cell(puzzle puz, struct step **stack, int x, int y, int last_guess) {
    int next = next_possibility(puz[x][y].u.pencil, last_guess);
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

int consistent(puzzle puz, int x, int y) {
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

int next_unfilled(puzzle puz, int *x, int *y) {
    while (*x < 9 && *y < 9 && puz[*x][*y].complete) {
        (*x)++;
        *y += *x / 9;
        *x %= 9;
    }
    return *x < 9 && *y < 9;
}

void test_stack(struct step *stack, struct step *end, puzzle puz) {
    while (stack < end) {
        dprintf("testing stack, getting pos x = %d, y = %d\n", stack->x, stack->y);
        assert(puz[stack->x][stack->y].complete);
        stack++;
    }
}

int run_backtrack(puzzle puz, struct step * const stack, struct step *stackp, int x, int y) {
    if (!next_unfilled(puz, &x, &y)) {
        return 1;
    }
    if(!fill_cell(puz, &stackp, x, y, 0)) {
        assert(0);
        return 0;
    }
    while (1) {
        dprintf("s = %ld, x = %d, y = %d\n", stackp - stack, x, y);
        assert(puz[x][y].complete);
        if (consistent(puz, x, y)) {
            if (!next_unfilled(puz, &x, &y)) {
                return 1;
            } else {
                dprintf("progressing\n");
                if (!fill_cell(puz, &stackp, x, y, 0)) {
                    return 0;
                }
            }
        } else {
            do {
                test_stack(stack, stackp, puz);
                dprintf("backtracking, x = %d, y = %d\n", x, y);
                assert(puz[x][y].complete);

                /* fill_cell increments stackp, so decrement to get last step
                 * written and prepare to overwrite the step*/
                stackp--;
                assert(stackp >= stack);
                assert(stackp->x == x && stackp->y == y);
                uint8_t last_tried = puz[x][y].u.ink;
                puz[x][y].complete = 0;
                puz[x][y].u.pencil = stackp->pencil;
                if (!fill_cell(puz, &stackp, x, y, last_tried)) {
                    /* we have exhausted options, so we must have guessed badly
                     * at some point before; therefore, we turn back */
                    if (stackp < stack) {
                        /* we have no where to turn back to, so fail out */
                        return 0;
                    } else {
                        /* back up even further, to 2 steps ago,
                         * change coordinates to set up for next round */
                        x = (stackp - 1)->x;
                        y = (stackp - 1)->y;
                        dprintf("backing up to x = %d, y = %d\n", x, y);
                        assert(x >= 0 && x < 9);
                        assert(y >= 0 && y < 9);
                        assert(puz[x][y].u.ink >= 1 && puz[x][y].u.ink <= 9);
                        assert(puz[x][y].complete);
                    }
                } else {
                    dprintf("success!\n");
                    break;
                }
            } while(1);
        }
    }
}

int backtrack(puzzle puz) {
    int stack_size = puzzle_noninked_count(puz);
    struct step stack[stack_size];
    dprintf("%d unfilled\n", stack_size);
    int success;
    if ((success = run_backtrack(puz, stack, stack, 0, 0))) {
        assert(puzzle_is_consistent(puz));
        assert(puzzle_noninked_count(puz) == 0);
    }
    return success;
}
