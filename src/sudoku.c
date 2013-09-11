#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "iter.h"
#include "debug.h"

/* constants */
#define ALL_POS 0x1ff
const char *sep = "-------------------------------------\n";
const uint16_t GROUPING[4] = { 0x5555, 0x3333, 0x0f0f, 0x00ff };

/* forward definitions */
void puzzle_print(puzzle puz, FILE *f);

/* helpers */
int hamming_weight(uint16_t x) {
    const uint16_t *g = GROUPING;
    for (int i = 1; i <= 8; i *= 2) {
        x = (x & *g) + ((x >> i) & *g);
        g++;
    }
    return x;
}

int puzzle_read(puzzle puz, FILE *f) {
    char line[64];
    int i, j;
    i = 0;
    while (fgets(line, sizeof(line), f)) {
        j = 0;
        while (j < 9 && line[j]) {
            char c = line[j];
            if (c == ' ') {
                puz[i][j].complete = 0;
                puz[i][j].u.pencil = ALL_POS;
            } else if (c >= '0' && c <= '9') {
                puz[i][j].complete = 1;
                puz[i][j].u.ink = c - '0';
            } else {
                return 0;
            }
            j++;
        }
        i++;
    }
    return 1;
}

void puzzle_pencil_possibilities(puzzle puz) {
    for (enum iter_type t = ROW; t <= BOX; t++) {
        for (int i = 0; i < 9; i++) {
            struct iter it;
            struct cell *c;
            uint16_t inked = 0;
            iter_init(&it, t, i);
            while ((c = iter_next(&it, puz))) {
                if (c->complete) {
                    inked |= 0x1 << (c->u.ink - 1);
                }
            }
            iter_init(&it, t, i);
            inked = ~inked;
            while ((c = iter_next(&it, puz))) {
                if (!c->complete) {
                    c->u.pencil &= inked;
                }
            }
        }
    }
}

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

void puzzle_print(puzzle puz, FILE *f) {
    struct cell *c;
    fputs(sep, f);
    for (int y = 0; y < 9; y++) {
        for (int p = 0; p < 3; p++) {
            for (int x = 0; x < 9; x++) {
                fputc('|', f);
                c = &puz[x][y];
                if (c->complete) {
                    if (p == 1) {
                        fprintf(f, "*%d*", c->u.ink);
                    } else {
                        fprintf(f, "***");
                    }
                } else {
                    for (int n = 3*p; n < 3 + 3*p; n++) {
                        if (c->u.pencil & (0x1 << n)) {
                            fprintf(f, "%d", n + 1);
                        } else {
                            fputc(' ', f);
                        }
                    }
                }
            }
            fputs("|\n", f);
        }
        fputs(sep, f);
    }
}

int puzzle_is_consistent(puzzle puz) {
    for (enum iter_type t = ROW; t <= BOX; t++) {
        for (int i = 0; i < 9; i++) {
            struct cell *c;
            struct iter it;
            uint16_t seen = 0;
            iter_init(&it, t, i);
            int j = 0;
            while ((c = iter_next(&it, puz))) {
                if (c->complete) {
                    uint16_t here = 0x1 << (c->u.ink - 1);
                    if (here & seen) {
                        dprintf("%s %d %d\n", iter_type_to_string[t], i, j++);
                        return 0;
                    } else {
                        seen |= here;
                    }
                }
            }
            /*
            if (seen != 0x1ff) {
                return 0;
            }
            */
        }
    }
    return 1;
}

int (*solve_strategies[])(puzzle) = {
    puzzle_singleton_cell,
    puzzle_singleton_number,
    puzzle_subgroup_exclusion_all
};

int strat_count = sizeof solve_strategies / sizeof solve_strategies[0];

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
    puzzle_solve(puz);
    puzzle_print(puz, stdout);
}
