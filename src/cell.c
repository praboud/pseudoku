#include <assert.h>
#include "cell.h"

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
