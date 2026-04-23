#ifndef BENCHMARK_FORMATS_H
#define BENCHMARK_FORMATS_H

#include <stdbool.h>

#define RESULTS_DIR "analisys/json_data"

typedef struct
{
    int columns;
    int rows;
    int seed;
    long long microseconds;
} TimeBenchmarkRow;

typedef struct
{
    int columns;
    int rows;
    int seed;
    bool all_pairs_connected;
    float branch_ratio;
    float dead_end_ratio;
    float corridor_ratio;
    float avg_degree;
    int diameter;
    double aspl;
    double entropy;
} TopologyReport;

#endif
