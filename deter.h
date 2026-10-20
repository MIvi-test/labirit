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

#ifndef LABIRIT_METRICS_ENABLED
#define LABIRIT_METRICS_ENABLED 1
#endif
#if LABIRIT_METRICS_ENABLED != 0 && LABIRIT_METRICS_ENABLED != 1
#error "LABIRIT_METRICS_ENABLED must be 0 or 1"
#endif

/* Disabled builds contain neither the local counter nor its increments. */
#if LABIRIT_METRICS_ENABLED
#define LABIRIT_STEPS_DECLARE() long long labirit_local_steps = 0
#define LABIRIT_STEPS_ADD(count) (labirit_local_steps += (count))
#define LABIRIT_STEPS_COMMIT() labirit_metrics_step(labirit_local_steps)
#else
#define LABIRIT_STEPS_DECLARE() ((void)0)
#define LABIRIT_STEPS_ADD(count) ((void)0)
#define LABIRIT_STEPS_COMMIT() ((void)0)
#endif

extern MazeGenerationMetrics LABIRIT_GENERATION_METRICS;

static inline void labirit_metrics_reset(void)
{
#if LABIRIT_METRICS_ENABLED
    LABIRIT_GENERATION_METRICS.steps = 0;
    LABIRIT_GENERATION_METRICS.current_memory_bytes = 0;
    LABIRIT_GENERATION_METRICS.peak_memory_bytes = 0;
#endif
}

static inline void labirit_metrics_reset_steps(void)
{
#if LABIRIT_METRICS_ENABLED
    LABIRIT_GENERATION_METRICS.steps = 0;
#endif
}

static inline void labirit_metrics_step(long long count)
{
#if LABIRIT_METRICS_ENABLED
    if (count > 0)
    {
        LABIRIT_GENERATION_METRICS.steps += count;
    }
#else
    (void)count;
#endif
}

static inline void labirit_metrics_alloc(size_t bytes)
{
#if LABIRIT_METRICS_ENABLED
    LABIRIT_GENERATION_METRICS.current_memory_bytes += bytes;
    if (LABIRIT_GENERATION_METRICS.current_memory_bytes > LABIRIT_GENERATION_METRICS.peak_memory_bytes)
    {
        LABIRIT_GENERATION_METRICS.peak_memory_bytes = LABIRIT_GENERATION_METRICS.current_memory_bytes;
    }
#else
    (void)bytes;
#endif
}

static inline void labirit_metrics_free(size_t bytes)
{
#if LABIRIT_METRICS_ENABLED
    if (bytes <= LABIRIT_GENERATION_METRICS.current_memory_bytes)
    {
        LABIRIT_GENERATION_METRICS.current_memory_bytes -= bytes;
    }
    else
    {
        LABIRIT_GENERATION_METRICS.current_memory_bytes = 0;
    }
#else
    (void)bytes;
#endif
}

static inline MazeGenerationMetrics labirit_metrics_snapshot(void)
{
#if LABIRIT_METRICS_ENABLED
    return LABIRIT_GENERATION_METRICS;
#else
    return (MazeGenerationMetrics){0};
#endif
}

#endif // DETER_H
