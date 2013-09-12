#include "cell.h"

#include "debug.h"
#include "puzzle.h"
#include "iter.h"

#define ALL_POS 0x1ff
const char *sep = "-------------------------------------\n";

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
            iter_mask(&it, puz, inked);
        }
    }
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

int puzzle_noninked_count(puzzle puz) {
    int count = 0;
    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 9; y++) {
            if (!puz[x][y].complete) count++;
        }
    }
    return count;
}
