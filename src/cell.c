#include <assert.h>
#include "cell.h"

uint8_t pencil_to_ink(uint16_t pencil) {
    assert(__builtin_popcount(pencil) == 1);
    return __builtin_ffs(pencil);
}

uint16_t ink_to_pencil(uint8_t ink) {
    return 0x1 << (ink - 1);
}

int pencil_contains_number(uint16_t pencil, uint8_t number) {
    return pencil & ink_to_pencil(number);
}

void pencil_print(uint16_t pencil, FILE* f) {
    putc('[', f);
    for (int i = 1; i <= 9; i++) {
        if (pencil_contains_number(pencil, i)) {
            putc('0' + i, f);
        }
    }
    putc(']', f);
}

uint16_t cell_coerce_pencil(struct cell *c) {
    if (c->complete) {
        return ink_to_pencil(c->u.ink);
    } else {
        return c->u.pencil;
    }
}

const uint16_t GROUPING[4] = { 0x5555, 0x3333, 0x0f0f, 0x00ff };

int hamming_weight(uint16_t x) {
    const uint16_t *g = GROUPING;
    for (int i = 1; i <= 8; i *= 2) {
        x = (x & *g) + ((x >> i) & *g);
        g++;
    }
    return x;
}
