#include "../backend/all_algorithms.h"
#include "../deter.h"
#include "benchmark_formats.h"
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

typedef void (*Generator)(MazeTable);

static int now_milliseconds(void)
{
    return (int)((clock() * 1000) / CLOCKS_PER_SEC);
}

bool ensure_results_dir(void)
{
    struct stat st;
    if (stat("analisys", &st) != 0 && mkdir("analisys", 0755) != 0)
    {
        return false;
    }
    if (stat(RESULTS_DIR, &st) == 0)
    {
        return true;
    }
    return mkdir(RESULTS_DIR, 0755) == 0;
}

static bool run_case(FILE *out, Generator generator, int columns, int rows, int seed, bool is_first)
{
    MazeTable table = init_table(columns, rows, seed);
    if (!table.data)
    {
        return false;
    }

    int started = now_milliseconds();
    generator(table);
    int finished = now_milliseconds();

    TimeBenchmarkRow row;
    row.columns = columns;
    row.rows = rows;
    row.seed = seed;
    row.milliseconds = finished - started;

    if (fprintf(out,
                "%s  {\"columns\":%d,\"rows\":%d,\"seed\":%d,\"milliseconds\":%d}",
                is_first ? "" : ",\n",
                row.columns, row.rows, row.seed, row.milliseconds) < 0)
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

    return true;
}
