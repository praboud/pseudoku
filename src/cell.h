#ifndef __CELL_H__
#define __CELL_H__

#include <stdint.h>
#include <stdio.h>

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

uint8_t pencil_to_ink(uint16_t pencil);
uint16_t ink_to_pencil(uint8_t ink);
int pencil_contains_number(uint16_t pencil, uint8_t number);
void pencil_print(uint16_t pencil, FILE* f);
uint16_t cell_coerce_pencil(struct cell *c);
int hamming_weight(uint16_t x);

#endif
