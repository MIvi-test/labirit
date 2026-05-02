#ifndef BENCHMARKS_RUNNER_H
#define BENCHMARKS_RUNNER_H

#include <stdbool.h>

bool ensure_results_dir(void);
bool run_combined_benchmarks(void);

static const char *CREATED_JSON_FILES[] = {
    "analisys/json_data/prim.json",
    "analisys/json_data/growing_tree.json",
    "analisys/json_data/watson.json",
    "analisys/json_data/dfs.json",
    "analisys/json_data/binary_tree.json",
    "analisys/json_data/recursive_division.json",
    "analisys/json_data/examples/maze_examples.json"};

static const int CREATED_JSON_FILES_COUNT = (int)(sizeof(CREATED_JSON_FILES) / sizeof(CREATED_JSON_FILES[0]));

#endif
