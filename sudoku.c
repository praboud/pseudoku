#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

/* constants */
#define ALL_POS 0x1ff
const char *sep = "-------------------------------------\n";
const uint16_t GROUPING[4] = { 0x5555, 0x3333, 0x0f0f, 0x00ff };

/* data definitions */
/* structure to represent contents of a single cell
 * can either be complete (with an inked/completed number)
 * or incomplete, with a bitstring representing the possible 
 * numbers which may go in this cell.
 */
struct cell {
    unsigned int complete : 1;
    union {
        uint16_t pencil; /* set of possible numbers, represented bitwise.
                            the rightmost bit represents 1,
                            the next represents 2, etc. each bit is lit iff
                            the corresponding number might be in this cell */
        uint8_t ink; /* completed (or inked) cell number,
                        valid iff completed == 1 */
    } u;
};

typedef struct cell puzzle[9][9];

enum iter_type { ROW, COL, BOX };
struct iter {
    enum iter_type type;
    int num; /* number of row, col, box */
    int pos; /* number of cell within group */
};

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

void iter_init(struct iter* i, enum iter_type t, int num) {
    i->type = t;
    i->num = num;
    i->pos = 0;
}

struct cell *iter_next(struct iter* i, puzzle puz) {
    if (i->pos == 9) {
        return NULL;
    }
    int x, y;
    switch (i->type) {
        case ROW:
            x = i->num;
            y = i->pos;
            break;
        case COL:
            x = i->pos;
            y = i->num;
            break;
        case BOX:
            x = (i->num % 3) * 3 + i->pos % 3;
            y = (i->num / 3) * 3 + i->pos / 3;
            break;
    }
    i->pos++;
    return &puz[x][y];
}

const char *iter_type_to_string[3] = { "ROW", "COL", "BOX" };

int puzzle_pencil_possibilities(puzzle puz) {
    for (enum iter_type t = ROW; t <= BOX; t++) {
        for (int i = 0; i < 9; i++) {
            struct iter it;
            struct cell *c;
            uint16_t inked = 0;
            iter_init(&it, t, i);
            while (c = iter_next(&it, puz)) {
                if (c->complete) {
                    inked |= 0x1 << (c->u.ink - 1);
                }
            }
            /* fprintf(stdout, "%s %d: %x\n", iter_type_to_string[t], i, inked); */
            iter_init(&it, t, i);
            inked = ~inked;
            while (c = iter_next(&it, puz)) {
                if (!c->complete) {
                    c->u.pencil &= inked;
                }
            }
        }
    }
}

int puzzle_singleton_cell(puzzle puz) {
    int change = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            struct cell *c = &puz[i][j];
            if (!c->complete && hamming_weight(c->u.pencil) == 1) {
                int x = __builtin_ctz(c->u.pencil);
                c->complete = 1;
                c->u.ink = x + 1;
            }
        }
    }
    if (change) puzzle_pencil_possibilities(puz);
    return change;
}

int puzzle_singleton_number(puzzle puz) {
    int change = 0;
    for (enum iter_type t = ROW; t <= BOX; t++) {
        for (int i = 0; i < 9; i++) {
            struct iter it;
            struct cell *c;
            uint16_t rst[9];
            memset(rst, 0, sizeof rst);
            iter_init(&it, t, i);
            int j = 0;
            while (c = iter_next(&it, puz)) {
                if (c->complete) {
                    for (int k = 0; k < 9; k++) rst[k] |= 0x1 << (c->u.ink - 1);
                } else {
                    for (int k = 0; k < j; k++) rst[k] |= c->u.pencil;
                    for (int k = j + 1; k < 9; k++) rst[k] |= c->u.pencil;
                }
                j++;
            }
            j = 0;
            iter_init(&it, t, i);
            while (c = iter_next(&it, puz)) {
                if (!c->complete) {
                    int x = c->u.pencil & ~rst[j];
                    /* fprintf(stderr, "%s %d, pos = %d, x = %d\n", iter_type_to_string[t], i, j, x); */
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
            while (c = iter_next(&it, puz)) {
                if (c->complete) {
                    uint16_t here = 0x1 << (c->u.ink - 1);
                    if (here & seen) {
                        fprintf(stderr, "%s %d %d\n", iter_type_to_string[t], i, j++);
                        return 0;
                    } else {
                        seen |= here;
                    }
                }
            }
            if (seen != 0x1ff) {
                return 0;
            }
        }
    }
    return 1;
}

int (*solve_strategies[2])(puzzle) = { puzzle_singleton_cell, puzzle_singleton_number };
#define STRAT_COUNT 2

void puzzle_solve(puzzle puz) {
    int change = 1;
    puzzle_pencil_possibilities(puz);
    while (change) {
        change = 0;
        for (int strat = 0; strat < STRAT_COUNT; strat++) {
            change |= solve_strategies[strat](puz);
        }
    }
    assert(puzzle_is_consistent(puz));
}

int main(char *argv[], int argc) {
    /* uint16_t x; */
    /* scanf("%hd", &x); */
    /* printf("%x, %d\n", x, hamming_weight(x)); */
    puzzle puz;
    puzzle_read(puz, stdin);
    puzzle_solve(puz);
    puzzle_print(puz, stdout);
}
