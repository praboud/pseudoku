#include <stdlib.h>
#include <assert.h>
#include "iter.h"

const char *iter_type_to_string[] = { "ROW", "COL", "BOX", "BOXT" };

void iter_init(struct iter* i, enum iter_type t, int num) {
    i->type = t;
    i->stype = ALL;
    i->num = num;
    i->pos = 0;
}

void iter_init_skip3(struct iter *i, enum iter_type t, int num, int skip) {
    i->type = t;
    i->stype = THREE;
    i->num = num;
    i->pos = 0;
    i->skip_start = skip;
}

struct cell *iter_next(struct iter* i, puzzle puz) {
    if (i->stype == THREE && i->skip_start == i->pos) {
        i->pos += 3;
    }
    if (i->pos == 9) {
        return NULL;
    }
    int x, y;
    switch (i->type) {
        case ROW:
            x = i->pos;
            y = i->num;
            break;
        case COL:
            x = i->num;
            y = i->pos;
            break;
        case BOX:
            x = (i->num % 3) * 3 + i->pos % 3;
            y = (i->num / 3) * 3 + i->pos / 3;
            break;
        case BOXT:
            x = (i->num / 3) * 3 + i->pos / 3;
            y = (i->num % 3) * 3 + i->pos % 3;
            break;
    }
    i->pos++;
    return &puz[x][y];
}

int iter_mask(struct iter* i, puzzle puz, uint16_t mask) {
    struct cell *c;
    int change = 0;
    uint16_t rev = ~mask;
    while ((c = iter_next(i, puz))) {
        if (!c->complete) {
            change = change || (mask & c->u.pencil);
            c->u.pencil &= rev;
        }
    }
    return change;
}

int iter_consistent(struct iter* i, puzzle puz) {
    uint16_t seen = 0;
    struct cell *c;
    int j = 0;
    while ((c = iter_next(i, puz))) {
        if (c->complete) {
            uint16_t cur = ink_to_pencil(c->u.ink);
            if (seen & cur) {
                return 0;
            } else {
                seen |= cur;
            }
        }
        j++;
    }
    return 1;
}
