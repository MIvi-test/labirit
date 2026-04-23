#include "../backend/all_algorithms.h"
#include "../deter.h"
#include "benchmark_formats.h"
#include <stdio.h>
#ifdef _WIN32
#include <direct.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <sys/stat.h>
#include <time.h>

typedef void (*Generator)(MazeTable);

static void dfs_adapter(MazeTable t)
{
    (void)dfs_algorithm(t);
}

static long long now_microseconds(void)
{
#ifdef _WIN32
    static LARGE_INTEGER freq;
    static int init = 0;
    if (!init)
    {
        QueryPerformanceFrequency(&freq);
        init = 1;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (long long)((counter.QuadPart * 1000000LL) / freq.QuadPart);
#else
    struct timespec ts;
#if defined(CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    timespec_get(&ts, TIME_UTC);
#endif
    return (long long)ts.tv_sec * 1000000LL + (long long)(ts.tv_nsec / 1000);
#endif
}

static int portable_mkdir(const char *path)
{
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

bool ensure_results_dir(void)
{
    struct stat st;
    if (stat("analisys", &st) != 0 && portable_mkdir("analisys") != 0)
    {
        return false;
    }
    if (stat(RESULTS_DIR, &st) == 0)
    {
        return true;
    }
    return portable_mkdir(RESULTS_DIR) == 0;
}

static bool run_case(FILE *out, Generator generator, int columns, int rows, int seed, bool is_first)
{
    MazeTable table = init_table(columns, rows, seed);
    if (!table.data)
    {
        return false;
    }

    long long started = now_microseconds();
    generator(table);
    long long finished = now_microseconds();

    TimeBenchmarkRow row;
    row.columns = columns;
    row.rows = rows;
    row.seed = seed;
    row.microseconds = finished - started;

    if (fprintf(out,
                "%s  {\"columns\":%d,\"rows\":%d,\"seed\":%d,\"microseconds\":%lld}",
                is_first ? "" : ",\n",
                row.columns, row.rows, row.seed, row.microseconds) < 0)
    {
        clear_table(&table);
        return false;
    }

    clear_table(&table);
    return true;
}

static bool benchmark_algorithm(const char *path, Generator generator,
                                const int *widths, size_t widths_count,
                                const int *heights, size_t heights_count,
                                const int *seeds, size_t seeds_count)
{
    FILE *out = fopen(path, "w");
    if (!out)
    {
        return false;
    }

    if (fprintf(out, "[\n") < 0)
    {
        fclose(out);
        return false;
    }

    bool first_entry = true;

    for (size_t w = 0; w < widths_count; w++)
    {
        for (size_t h = 0; h < heights_count; h++)
        {
            for (size_t s = 0; s < seeds_count; s++)
            {
                if (!run_case(out, generator, widths[w], heights[h], seeds[s], first_entry))
                {
                    fclose(out);
                    return false;
                }
                first_entry = false;
            }
        }
    }

    if (fprintf(out, "\n]\n") < 0)
    {
        fclose(out);
        return false;
    }

    fclose(out);
    return true;
}

bool run_time_benchmarks(void)
{
    const int common_widths[] = {8, 16, 32, 64, 96, 128};
    const int common_heights[] = {8, 12, 24, 48, 72, 96};
    const int seeds[] = {101, 202, 303, 404, 505};

    if (!ensure_results_dir())
    {
        return false;
    }

    if (!benchmark_algorithm(RESULTS_DIR "/timewait_prim.json", prim_alg,
                             common_widths, sizeof(common_widths) / sizeof(common_widths[0]),
                             common_heights, sizeof(common_heights) / sizeof(common_heights[0]),
                             seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }
    if (!benchmark_algorithm(RESULTS_DIR "/timewait_growing_tree.json", growing_tree_alg,
                             common_widths, sizeof(common_widths) / sizeof(common_widths[0]),
                             common_heights, sizeof(common_heights) / sizeof(common_heights[0]),
                             seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }
    if (!benchmark_algorithm(RESULTS_DIR "/timewait_watson.json", watson_alg,
                             common_widths, sizeof(common_widths) / sizeof(common_widths[0]),
                             common_heights, sizeof(common_heights) / sizeof(common_heights[0]),
                             seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }

    if (!benchmark_algorithm(RESULTS_DIR "/timewait_dfs.json", dfs_adapter,
                             common_widths, sizeof(common_widths) / sizeof(common_widths[0]),
                             common_heights, sizeof(common_heights) / sizeof(common_heights[0]),
                             seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }
    if (!benchmark_algorithm(RESULTS_DIR "/timewait_binary_tree.json", binary_algos,
                             common_widths, sizeof(common_widths) / sizeof(common_widths[0]),
                             common_heights, sizeof(common_heights) / sizeof(common_heights[0]),
                             seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }
    if (!benchmark_algorithm(RESULTS_DIR "/timewait_recursive_division.json", recursive_division_algorithm,
                             common_widths, sizeof(common_widths) / sizeof(common_widths[0]),
                             common_heights, sizeof(common_heights) / sizeof(common_heights[0]),
                             seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }

    return true;
}
