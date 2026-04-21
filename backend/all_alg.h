#ifndef ALGOS_FOR_CREATE_LABIRINT
#define ALGOS_FOR_CREATE_LABIRINT

#include "../deter.h"

TABLE INIT_TABLE(unsigned int columns, unsigned int rows, unsigned int seed);
bool DFS_alg(TABLE table);
void binary_algos(TABLE table);
void reqursive_alg(TABLE table);
void prim_alg(TABLE table);
void watson_alg(TABLE table);
void growing_tree_alg(TABLE table);

#include "binary_tree_alg.c"
#include "DFS_alg.c"
#include "req_division.c"
#include "init.c"
#include "prim.c"
#include "watson_alg.c"
#include "growing_tree_alg.c"

#endif