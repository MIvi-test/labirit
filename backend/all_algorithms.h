#ifndef ALGORITHMS_FOR_MAZE_GENERATION_H
#define ALGORITHMS_FOR_MAZE_GENERATION_H

#include "../deter.h"

MazeTable init_table(unsigned int columns, unsigned int rows, unsigned int seed);
MazeTable init_table_open(unsigned int columns, unsigned int rows, unsigned int seed);
bool dfs_algorithm(MazeTable table);
void binary_algos(MazeTable table);
void recursive_division_algorithm(MazeTable table);
/* Only for a fresh init_table_open result. */
void recursive_division_prepared(MazeTable table);
void prim_alg(MazeTable table);
void watson_alg(MazeTable table);
void growing_tree_alg(MazeTable table);
void clear_table(MazeTable *table);

#endif
