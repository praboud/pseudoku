#ifndef __ITER_H__
#define __ITER_H__

#include "cell.h"

enum iter_type { ROW, COL, BOX, BOXT };
enum iter_skip_type { ALL, THREE, ARRAY };
struct iter {
    enum iter_type type;
    enum iter_skip_type stype;
    int num; /* number of row, col, box */
    int pos; /* number of cell within group */
    int skip_start;
};

struct coord {
    int x;
    int y;
};

void iter_init(struct iter* i, enum iter_type t, int num);
void iter_init_skip3(struct iter *i, enum iter_type t, int num, int skip);
struct cell *iter_next(struct iter* i, puzzle puz);
struct cell *iter_next_c(struct iter* i, puzzle puz, struct coord *c);
int iter_mask(struct iter* i, puzzle puz, uint16_t mask);
int iter_consistent(struct iter* i, puzzle puz);
extern const char *iter_type_to_string[];

#endif
