#ifndef ALGORITHMS_FOR_MAZE_GENERATION_H
#define ALGORITHMS_FOR_MAZE_GENERATION_H

#include "../deter.h"

MazeTable init_table(unsigned int columns, unsigned int rows, unsigned int seed);
bool dfs_algorithm(MazeTable table);
void binary_algos(MazeTable table);
void recursive_division_algorithm(MazeTable table);
void prim_alg(MazeTable table);
void watson_alg(MazeTable table);
void growing_tree_alg(MazeTable table);
void clear_table(MazeTable *table);

#include "binary_tree_alg.c"
#include "dfs_algorithm.c"
#include "recursive_division_algorithm.c"
#include "table_init.c"
#include "prim.c"
#include "watson_alg.c"
#include "growing_tree_alg.c"

#endif
