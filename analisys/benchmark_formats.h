#ifndef BENCHMARK_FORMATS_H
#define BENCHMARK_FORMATS_H

#include <stdbool.h>
#include <stddef.h>

#define RESULTS_DIR "analisys/json_data"

/* Global benchmark constraints (requested):
   - linear sweep changes columns OR rows by +4 each step
   - max grid is 100x100
*/
#define BENCH_DIM_MAX 100

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
} BenchmarkGridSize;

typedef struct
{
    int columns;
    int rows;
    int seed;
    bool all_pairs_connected;
    float branch_ratio;
    long long steps_count;
    size_t memory_peak_bytes;
    float wall_density;
    int dead_end_count;
    float dead_end_ratio;
    float corridor_ratio;
    float avg_degree;
    int cycle_count;
    int diameter;
    int longest_path;
    double symmetry_score;
    double fractal_dimension;
    double aspl;
    double entropy;
} TopologyReport;

/*
   Topology benchmarks:
   Linear growth (rows += 4) with bounded size.
   Constraint: |columns-rows| <= max(columns, rows)/2 (per-grid).
   Note: ASPL/entropy slow-path is guarded at 4096 cells in code.
*/
static const BenchmarkGridSize TOPOLOGY_LINEAR_CELL_CASES[] = {
    /* fixed columns = 32, rows from 16 to 64 with step 4 */
    {32, 16}, {32, 20}, {32, 24}, {32, 28}, {32, 32},
    {32, 36}, {32, 40}, {32, 44}, {32, 48}, {32, 52},
    {32, 56}, {32, 60}, {32, 64}};

/*
   Time benchmarks:
   - Linear size increase by 4 (rows += 4) with one fixed dimension.
   - For each chosen `cells = columns*rows`, provide exactly 6 (columns, rows)
     combinations (ordered, i.e. mirrored pairs count) to plot 6 aspect-ratio
     lines per cell-count facet.
   - Max grid: 100x100.
   - Requested balance (per-grid): |columns-rows| <= max(columns, rows)/2.
*/
static const BenchmarkGridSize TIME_LINEAR_STEP_CASES[] = {
    /* fixed columns = 100, rows from 50 to 100 with step 4 (balance rule holds) */
    {100, 50}, {100, 54}, {100, 58}, {100, 62}, {100, 66}, {100, 70},
    {100, 74}, {100, 78}, {100, 82}, {100, 86}, {100, 90}, {100, 94},
    {100, 98}, {100, 100}};

static const BenchmarkGridSize SAME_CELLS_ASPECT_RATIO_CASES[] = {
    /* cells = 840 (6 cases, balanced) */
    {21, 40}, {24, 35}, {28, 30}, {30, 28}, {35, 24}, {40, 21},

    /* cells = 1440 (6 cases, balanced) */
    {30, 48}, {32, 45}, {36, 40}, {40, 36}, {45, 32}, {48, 30},

    /* cells = 2016 (6 cases, balanced) */
    {32, 63}, {36, 56}, {42, 48}, {48, 42}, {56, 36}, {63, 32},

    /* cells = 2880 (6 cases, balanced) */
    {40, 72}, {45, 64}, {48, 60}, {60, 48}, {64, 45}, {72, 40}};

/*
   Combined time-benchmark sizes used by C runner:
   linear squares + aspect-ratio cases.
*/
static const BenchmarkGridSize TIME_BENCH_CASES[] = {
    /* linear sweep */
    {100, 50}, {100, 54}, {100, 58}, {100, 62}, {100, 66}, {100, 70},
    {100, 74}, {100, 78}, {100, 82}, {100, 86}, {100, 90}, {100, 94},
    {100, 98}, {100, 100},

    /* aspect-ratio sweep (grouped by cells) */
    {21, 40}, {24, 35}, {28, 30}, {30, 28}, {35, 24}, {40, 21},
    {30, 48}, {32, 45}, {36, 40}, {40, 36}, {45, 32}, {48, 30},
    {32, 63}, {36, 56}, {42, 48}, {48, 42}, {56, 36}, {63, 32},
    {40, 72}, {45, 64}, {48, 60}, {60, 48}, {64, 45}, {72, 40}};

#endif
