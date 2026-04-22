#ifndef BENCHMARKS_RUNNER_H
#define BENCHMARKS_RUNNER_H

#include <stdbool.h>

bool ensure_results_dir(void);
bool run_time_benchmarks(void);
bool run_topology_benchmarks(void);

static const char *CREATED_JSON_FILES[] = {
    "analisys/json_data/timewait_prim.json",
    "analisys/json_data/timewait_growing_tree.json",
    "analisys/json_data/timewait_watson.json",
    "analisys/json_data/topology_prim.json",
    "analisys/json_data/topology_growing_tree.json",
    "analisys/json_data/topology_watson.json"};

static const int CREATED_JSON_FILES_COUNT = (int)(sizeof(CREATED_JSON_FILES) / sizeof(CREATED_JSON_FILES[0]));

#endif
