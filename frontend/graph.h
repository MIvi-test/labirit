#ifndef LABIRIT_FRONTEND_GRAPH_H
#define LABIRIT_FRONTEND_GRAPH_H

#include "../deter.h"

typedef enum
{
    MAZE_ALGO_PRIM = 0,
    MAZE_ALGO_DFS,
    MAZE_ALGO_GROWING_TREE,
    MAZE_ALGO_WATSON,
    MAZE_ALGO_BINARY
} MazeAlgorithm;

MazeAlgorithm parse_algorithm_name(const char *name);
const char *maze_algorithm_name(MazeAlgorithm algorithm);
void maze_run_algorithm(TABLE table, MazeAlgorithm algorithm);
void print_maze(TABLE table);
void print_maze_info(TABLE table, MazeAlgorithm algorithm);
int run_maze_window(int columns, int rows, unsigned int seed, MazeAlgorithm algorithm);

#include "graph.c"

#endif
