cmake_minimum_required(VERSION 2.6)
set(CURSES_LIBRARY /usr/lib/libcurses.so)


project(pseudoku)
add_definitions(-std=c99)
add_definitions(-W)
add_definitions(-Wall)
add_definitions(-Wextra)
add_definitions(-pedantic)
add_definitions(-g)
add_definitions(-O2)

add_library(pseudoku src/iter.c src/cell.c src/puzzle.c src/strategy.c src/backtrack.c src/generator.c)

add_executable(solver src/sudoku.c)
add_executable(interactive src/interactive.c)

target_link_libraries(interactive pseudoku ${CURSES_LIBRARY} ${LIBS})
target_link_libraries(solver pseudoku ${LIBS})
