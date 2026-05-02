#ifndef DETER_H
#define DETER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

typedef union
{
    unsigned char all_bits;
    struct
    {
        unsigned char top : 1;
        unsigned char right : 1;
        unsigned char bottom : 1;
        unsigned char left : 1;
        unsigned char unused : 4;
    } wall;
} Cell;

typedef struct
{
    Cell **data;
    int rows;
    int columns;
    int seed;

} TABLE;

typedef TABLE MazeTable;

typedef struct
{
    long long steps;
    size_t current_memory_bytes;
    size_t peak_memory_bytes;
} MazeGenerationMetrics;

static MazeGenerationMetrics LABIRIT_GENERATION_METRICS = {0, 0, 0};

static inline void labirit_metrics_reset(void)
{
    LABIRIT_GENERATION_METRICS.steps = 0;
    LABIRIT_GENERATION_METRICS.current_memory_bytes = 0;
    LABIRIT_GENERATION_METRICS.peak_memory_bytes = 0;
}

static inline void labirit_metrics_reset_steps(void)
{
    LABIRIT_GENERATION_METRICS.steps = 0;
}

static inline void labirit_metrics_step(long long count)
{
    if (count > 0)
    {
        LABIRIT_GENERATION_METRICS.steps += count;
    }
}

static inline void labirit_metrics_alloc(size_t bytes)
{
    LABIRIT_GENERATION_METRICS.current_memory_bytes += bytes;
    if (LABIRIT_GENERATION_METRICS.current_memory_bytes > LABIRIT_GENERATION_METRICS.peak_memory_bytes)
    {
        LABIRIT_GENERATION_METRICS.peak_memory_bytes = LABIRIT_GENERATION_METRICS.current_memory_bytes;
    }
}

static inline void labirit_metrics_free(size_t bytes)
{
    if (bytes <= LABIRIT_GENERATION_METRICS.current_memory_bytes)
    {
        LABIRIT_GENERATION_METRICS.current_memory_bytes -= bytes;
    }
    else
    {
        LABIRIT_GENERATION_METRICS.current_memory_bytes = 0;
    }
}

static inline MazeGenerationMetrics labirit_metrics_snapshot(void)
{
    return LABIRIT_GENERATION_METRICS;
}

#endif // DETER_H
