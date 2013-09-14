#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "debug.h"
#include "strategy.h"
#include "puzzle.h"
#include "backtrack.h"
#include "constants.h"
#include "generator.h"

void _puzzle_printw_cell(struct cell *c, int highlight) {
    int x, y;
    getyx(stdscr, y, x);
    if (highlight && (cell_coerce_pencil(c) & ink_to_pencil(highlight))) {
        attron(COLOR_PAIR(2));
    }
    if (!c->complete) {
        for (int i = INK_START; i <= INK_END; i += 3) {
            move(y++, x);
            for (int j = 0; j < 3; j++) {
                addch((c->u.pencil & ink_to_pencil(i + j)) ? ('0' + i + j) : ' ');
            }
        }
    } else {
        move(y++, x);
        addstr("***");
        move(y++, x);
        printw("*%c*", '0' + c->u.ink);
        move(y++, x);
        addstr("***");
    }
    move(y, x);
    attroff(COLOR_PAIR(2));
}

void _print_dividers(int x, int y) {
    /* x and y are the selected cell's coordinates */
    /* i and j are the coordinates corresponding to x and y respectively */
    /* print horizontal dividers */
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j <= 9; j++) {
            if (x == i && (y == j || y == j - 1)) {
                attron(A_REVERSE);
            }
            if (j % 3 == 0) {
                attron(COLOR_PAIR(1));
            }
            mvaddstr(4 * j, 1 + 4 * i, "---");
            attroff(A_REVERSE);
            attroff(COLOR_PAIR(1));
        }
    }

    /* print vertical dividers */
    for (int i = 0; i <= 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (i % 3 == 0) {
                attron(COLOR_PAIR(1));
            }
            if (y == j && (x == i || x == i - 1)) {
                attron(A_REVERSE);
            }
            mvaddch(1 + 4 * j, 4 * i, '|');
            mvaddch(2 + 4 * j, 4 * i, '|');
            mvaddch(3 + 4 * j, 4 * i, '|');
            attroff(A_REVERSE);
            attroff(COLOR_PAIR(1));
        }
    }
    return;
    /* print vertical dividers */
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (y == j && (x == i || x == i + 1)) {
                attron(A_BOLD);
            } else {
                attroff(A_BOLD);
            }
            mvaddch(1 + 3 * j, 1 + 3 * i, '|');
            mvaddch(2 + 3 * j, 1 + 3 * i, '|');
            mvaddch(3 + 3 * j, 1 + 3 * i, '|');
        }
    }
}

void _puzzle_printw(puzzle puz, int highlight) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            move(1 + 4 * i, 1 + 4 * j);
            _puzzle_printw_cell(&puz[j][i], highlight);
        }
    }
}

void _init_scr(void) {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    int x, y;
    getmaxyx(stdscr, y, x);
    if (x < 37 || y < 37) {
        endwin();
        printf("Screen is too small\n");
        exit(1);
    } else if (has_colors() == FALSE) {
        endwin();
        printf("No color support\n");
        exit(1);
    }
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
}

void _toggle_cell(puzzle p, int x, int y, int n) {
    if (!p[x][y].complete) {
        p[x][y].u.pencil ^= ink_to_pencil(n);
    }
}

void interactive(void) {
    puzzle puz;
    /* FILE *f = fopen("p2", "r"); */
    /* puzzle_read(puz, f); */
    puzzle_generate(puz);
    puzzle_pencil_possibilities(puz);
    /* fclose(f); */
    _init_scr();
    int ch;
    int x = 4;
    int y = 4;
    int highlight = 0;
    _puzzle_printw(puz, 0);
    _print_dividers(x, y);
    refresh();
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case 'h': x--;
                break;
            case 's': x++;
                break;
            case 't': y++;
                break;
            case 'n': y--;
                break;
            case 'c':
                ch = getch();
                if ('0' <= ch && ch <= '9') {
                    highlight = ch - '0';
                } else {
                    highlight = 0;
                }
                break;
            case 'f':
                ch = getch();
                if ('0' <= ch && ch <= '9') {
                    if (!puz[x][y].complete) {
                        puzzle_fill_cell(puz, x, y, ch - '0');
                    } else {
                        puzzle_clear_cell(puz, x, y);
                    }
                }
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                _toggle_cell(puz, x, y, ch - '0');
                break;
        }
        _puzzle_printw(puz, highlight);
        _print_dividers(x, y);
        refresh();
    }
    endwin();
}
