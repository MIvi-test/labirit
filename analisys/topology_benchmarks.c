#include "../backend/all_algorithms.h"
#include "../deter.h"
#include "benchmark_formats.h"
#include "distance_metrics.h" // Используем ту же реализацию, что и combined_benchmarks.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef void (*Generator)(MazeTable);

static void dfs_topology_adapter(MazeTable t)
{
    (void)dfs_algorithm(t);
}

static int cell_degree(MazeTable table, int x, int y)
{
    int degree = 0;
    if (table.data[y][x].wall.top == 0)
        degree++;
    if (table.data[y][x].wall.right == 0)
        degree++;
    if (table.data[y][x].wall.bottom == 0)
        degree++;
    if (table.data[y][x].wall.left == 0)
        degree++;
    return degree;
}

static int bfs_farthest(MazeTable table, int start, int *dist_out)
{
    int total = table.rows * table.columns;
    int *queue = (int *)malloc(sizeof(int) * total);
    int *dist = (int *)malloc(sizeof(int) * total);
    if (!queue || !dist)
    {
        free(queue);
        free(dist);
        return start;
    }

    for (int i = 0; i < total; i++)
    {
        dist[i] = -1;
    }

    int head = 0;
    int tail = 0;
    queue[tail++] = start;
    dist[start] = 0;
    int farthest = start;

    while (head < tail)
    {
        int cur = queue[head++];
        int x = cur % table.columns;
        int y = cur / table.columns;
        if (dist[cur] > dist[farthest])
        {
            farthest = cur;
        }

        int neighbors[4];
        int count = 0;
        if (table.data[y][x].wall.top == 0 && y > 0)
            neighbors[count++] = (y - 1) * table.columns + x;
        if (table.data[y][x].wall.right == 0 && x < table.columns - 1)
            neighbors[count++] = y * table.columns + x + 1;
        if (table.data[y][x].wall.bottom == 0 && y < table.rows - 1)
            neighbors[count++] = (y + 1) * table.columns + x;
        if (table.data[y][x].wall.left == 0 && x > 0)
            neighbors[count++] = y * table.columns + x - 1;

        for (int i = 0; i < count; i++)
        {
            int next = neighbors[i];
            if (dist[next] == -1)
            {
                dist[next] = dist[cur] + 1;
                queue[tail++] = next;
            }
        }
    }

    if (dist_out)
    {
        *dist_out = dist[farthest];
    }

    free(dist);
    free(queue);
    return farthest;
}

static bool bfs_connected(MazeTable table, int start)
{
    int total = table.rows * table.columns;
    int *queue = (int *)malloc(sizeof(int) * total);
    int *dist = (int *)malloc(sizeof(int) * total);
    if (!queue || !dist)
    {
        free(queue);
        free(dist);
        return false;
    }

    for (int i = 0; i < total; i++)
    {
        dist[i] = -1;
    }

    int head = 0;
    int tail = 0;
    queue[tail++] = start;
    dist[start] = 0;

    while (head < tail)
    {
        int cur = queue[head++];
        int x = cur % table.columns;
        int y = cur / table.columns;

        int neighbors[4];
        int count = 0;
        if (table.data[y][x].wall.top == 0 && y > 0)
            neighbors[count++] = (y - 1) * table.columns + x;
        if (table.data[y][x].wall.right == 0 && x < table.columns - 1)
            neighbors[count++] = y * table.columns + x + 1;
        if (table.data[y][x].wall.bottom == 0 && y < table.rows - 1)
            neighbors[count++] = (y + 1) * table.columns + x;
        if (table.data[y][x].wall.left == 0 && x > 0)
            neighbors[count++] = y * table.columns + x - 1;

        for (int i = 0; i < count; i++)
        {
            int next = neighbors[i];
            if (dist[next] == -1)
            {
                dist[next] = dist[cur] + 1;
                queue[tail++] = next;
            }
        }
    }

    bool ok = true;
    for (int i = 0; i < total; i++)
    {
        if (dist[i] == -1)
        {
            ok = false;
            break;
        }
    }

    free(dist);
    free(queue);
    return ok;
}

static bool analyze_topology(MazeTable table, TopologyReport *out, int columns, int rows, int seed)
{
    if (!out || !table.data)
    {
        return false;
    }

    int total = rows * columns;
    int branch = 0;
    int dead_end = 0;
    int corridor = 0;
    int degree_sum = 0;

    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < columns; x++)
        {
            int degree = cell_degree(table, x, y);
            degree_sum += degree;
            if (degree >= 3)
                branch++;
            if (degree == 1)
                dead_end++;
            if (degree == 2)
                corridor++;
        }
    }

    int dist = 0;
    int a = bfs_farthest(table, 0, &dist);
    int diameter = 0;
    (void)bfs_farthest(table, a, &diameter);

    out->columns = columns;
    out->rows = rows;
    out->seed = seed;
    out->all_pairs_connected = bfs_connected(table, 0);
    out->branch_ratio = (float)branch / (float)total;
    out->dead_end_ratio = (float)dead_end / (float)total;
    out->corridor_ratio = (float)corridor / (float)total;
    out->avg_degree = (float)degree_sum / (float)total;
    out->diameter = diameter;
    DistanceMetrics distances = {-1.0, -1.0}; // Значения для несвязного графа.
    if (out->all_pairs_connected) // Расстояния между всеми парами должны существовать.
        distances = compute_distance_metrics(table); // Один расчёт вместо двух наборов BFS.
    out->aspl = distances.aspl; // Записываем среднее расстояние в отчёт.
    out->entropy = distances.entropy; // Записываем энтропию в отчёт.
    return true;
}

static bool benchmark_topology_one(const char *path, Generator generator,
                                   const BenchmarkGridSize *sizes,
                                   size_t sizes_count,
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

    for (size_t i = 0; i < sizes_count; i++)
    {
        for (size_t s = 0; s < seeds_count; s++)
        {
            int columns = sizes[i].columns;
            int rows = sizes[i].rows;
            int seed = seeds[s];
            MazeTable table = generator == recursive_division_algorithm
                                  ? init_table_open(columns, rows, seed)
                                  : init_table(columns, rows, seed);
            if (!table.data)
            {
                fclose(out);
                return false;
            }

            if (generator == recursive_division_algorithm)
                recursive_division_prepared(table);
            else
                generator(table);
            TopologyReport report;
            if (!analyze_topology(table, &report, columns, rows, seed))
            {
                clear_table(&table);
                fclose(out);
                return false;
            }
            if (fprintf(out,
                        "%s  {\"columns\":%d,\"rows\":%d,\"seed\":%d,"
                        "\"all_pairs_connected\":%s,\"branch_ratio\":%.6f,"
                        "\"dead_end_ratio\":%.6f,\"corridor_ratio\":%.6f,"
                        "\"avg_degree\":%.6f,\"diameter\":%d,\"aspl\":%.6f,\"entropy\":%.6f}",
                        first_entry ? "" : ",\n",
                        report.columns, report.rows, report.seed,
                        report.all_pairs_connected ? "true" : "false",
                        report.branch_ratio, report.dead_end_ratio,
                        report.corridor_ratio, report.avg_degree,
                        report.diameter, report.aspl, report.entropy) < 0)
            {
                clear_table(&table);
                fclose(out);
                return false;
            }
            first_entry = false;

            clear_table(&table);
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

bool run_topology_benchmarks(void)
{
    const int seeds[] = {101, 202, 303, 404, 505, 606, 707, 808, 909, 1010};

    if (!benchmark_topology_one(RESULTS_DIR "/topology_prim.json", prim_alg,
                                TIME_BENCH_CASES,
                                sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }
    if (!benchmark_topology_one(RESULTS_DIR "/topology_growing_tree.json", growing_tree_alg,
                                TIME_BENCH_CASES,
                                sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }
    if (!benchmark_topology_one(RESULTS_DIR "/topology_watson.json", watson_alg,
                                TIME_BENCH_CASES,
                                sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }

    if (!benchmark_topology_one(RESULTS_DIR "/topology_dfs.json", dfs_topology_adapter,
                                TIME_BENCH_CASES,
                                sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }
    if (!benchmark_topology_one(RESULTS_DIR "/topology_binary_tree.json", binary_algos,
                                TIME_BENCH_CASES,
                                sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }
    if (!benchmark_topology_one(RESULTS_DIR "/topology_recursive_division.json", recursive_division_algorithm,
                                TIME_BENCH_CASES,
                                sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                seeds, sizeof(seeds) / sizeof(seeds[0])))
    {
        return false;
    }

    return true;
}
