#ifndef ANALISYS_SIMPLE_PATHS_H
#define ANALISYS_SIMPLE_PATHS_H

#include "../deter.h"
#include <stdint.h>

/*
 * Counts simple paths (no repeated vertices) between (sx,sy) and (tx,ty)
 * in the already generated maze graph.
 *
 * This is an intentionally slow baseline:
 * - Backtracking DFS over all simple paths (can be exponential).
 * - Optional max_len to prune long paths (-1 means no length limit).
 * - Optional cap to stop counting after reaching cap (0 means "no cap").
 *
 * Returns the number of paths found (possibly capped).
 */
uint64_t count_simple_paths_slow(MazeTable table,
                                int sx, int sy,
                                int tx, int ty,
                                int max_len,
                                uint64_t cap);

#endif

