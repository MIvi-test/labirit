#include "../backend/all_algorithms.h"
#include "../deter.h"
#include "benchmark_formats.h"
#include <stdio.h>
#include <math.h>
#ifdef _WIN32
#include <direct.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <sys/stat.h>
#include <time.h>

typedef void (*Generator)(MazeTable);

static void dfs_topology_adapter(MazeTable t)
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

static bool ensure_examples_dir(void)
{
    struct stat st;
    const char *path = RESULTS_DIR "/examples";
    if (stat(path, &st) == 0)
    {
        return true;
    }
    return portable_mkdir(path) == 0;
}

/* --- Topology analysis functions: copied from topology_benchmarks.c --- */

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

static int count_open_edges(MazeTable table)
{
    int edges = 0;
    for (int y = 0; y < table.rows; y++)
    {
        for (int x = 0; x < table.columns; x++)
        {
            if (x + 1 < table.columns && table.data[y][x].wall.right == 0)
            {
                edges++;
            }
            if (y + 1 < table.rows && table.data[y][x].wall.bottom == 0)
            {
                edges++;
            }
        }
    }
    return edges;
}

static float compute_wall_density(MazeTable table)
{
    int closed = 0;
    int total_sides = table.rows * table.columns * 4;
    for (int y = 0; y < table.rows; y++)
    {
        for (int x = 0; x < table.columns; x++)
        {
            closed += table.data[y][x].wall.top ? 1 : 0;
            closed += table.data[y][x].wall.right ? 1 : 0;
            closed += table.data[y][x].wall.bottom ? 1 : 0;
            closed += table.data[y][x].wall.left ? 1 : 0;
        }
    }
    return total_sides > 0 ? (float)closed / (float)total_sides : 0.0f;
}

static int count_components(MazeTable table)
{
    int total = table.rows * table.columns;
    bool *visited = (bool *)calloc((size_t)total, sizeof(bool));
    int *queue = (int *)malloc(sizeof(int) * (size_t)total);
    if (!visited || !queue)
    {
        free(visited);
        free(queue);
        return 0;
    }

    int components = 0;
    for (int start = 0; start < total; start++)
    {
        if (visited[start])
        {
            continue;
        }
        components++;
        int head = 0;
        int tail = 0;
        queue[tail++] = start;
        visited[start] = true;

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
                if (!visited[next])
                {
                    visited[next] = true;
                    queue[tail++] = next;
                }
            }
        }
    }

    free(queue);
    free(visited);
    return components;
}

static unsigned char wall_bits(Cell cell)
{
    return (unsigned char)((cell.wall.top ? 1 : 0) |
                           ((cell.wall.right ? 1 : 0) << 1) |
                           ((cell.wall.bottom ? 1 : 0) << 2) |
                           ((cell.wall.left ? 1 : 0) << 3));
}

static unsigned char transform_wall_bits(unsigned char bits, int transform)
{
    bool top = (bits & 1) != 0;
    bool right = (bits & 2) != 0;
    bool bottom = (bits & 4) != 0;
    bool left = (bits & 8) != 0;

    if (transform == 0)
    {
        return (unsigned char)((top ? 1 : 0) | ((left ? 1 : 0) << 1) |
                               ((bottom ? 1 : 0) << 2) | ((right ? 1 : 0) << 3));
    }
    if (transform == 1)
    {
        return (unsigned char)((bottom ? 1 : 0) | ((right ? 1 : 0) << 1) |
                               ((top ? 1 : 0) << 2) | ((left ? 1 : 0) << 3));
    }
    return (unsigned char)((bottom ? 1 : 0) | ((left ? 1 : 0) << 1) |
                           ((top ? 1 : 0) << 2) | ((right ? 1 : 0) << 3));
}

static int bit_similarity_count(unsigned char a, unsigned char b)
{
    int matches = 0;
    for (int bit = 0; bit < 4; bit++)
    {
        if (((a >> bit) & 1) == ((b >> bit) & 1))
        {
            matches++;
        }
    }
    return matches;
}

static double compute_symmetry_score(MazeTable table)
{
    long long matches = 0;
    long long comparisons = (long long)table.rows * (long long)table.columns * 12LL;
    for (int y = 0; y < table.rows; y++)
    {
        for (int x = 0; x < table.columns; x++)
        {
            unsigned char original = wall_bits(table.data[y][x]);
            matches += bit_similarity_count(original, transform_wall_bits(wall_bits(table.data[y][table.columns - 1 - x]), 0));
            matches += bit_similarity_count(original, transform_wall_bits(wall_bits(table.data[table.rows - 1 - y][x]), 1));
            matches += bit_similarity_count(original, transform_wall_bits(wall_bits(table.data[table.rows - 1 - y][table.columns - 1 - x]), 2));
        }
    }
    return comparisons > 0 ? (double)matches / (double)comparisons : 0.0;
}

static bool wall_point_occupied(MazeTable table, int gx, int gy)
{
    int width = table.columns * 2 + 1;
    int height = table.rows * 2 + 1;
    if (gx <= 0 || gy <= 0 || gx >= width - 1 || gy >= height - 1)
    {
        return true;
    }
    if ((gy % 2) == 0 && (gx % 2) == 1)
    {
        int x = gx / 2;
        int y = gy / 2;
        return table.data[y][x].wall.top != 0;
    }
    if ((gx % 2) == 0 && (gy % 2) == 1)
    {
        int x = gx / 2;
        int y = gy / 2;
        return table.data[y][x].wall.left != 0;
    }
    if ((gx % 2) == 0 && (gy % 2) == 0)
    {
        return wall_point_occupied(table, gx - 1, gy) ||
               wall_point_occupied(table, gx + 1, gy) ||
               wall_point_occupied(table, gx, gy - 1) ||
               wall_point_occupied(table, gx, gy + 1);
    }
    return false;
}

static int count_wall_boxes(MazeTable table, int box_size)
{
    int width = table.columns * 2 + 1;
    int height = table.rows * 2 + 1;
    int boxes = 0;
    for (int y0 = 0; y0 < height; y0 += box_size)
    {
        for (int x0 = 0; x0 < width; x0 += box_size)
        {
            bool occupied = false;
            for (int y = y0; y < y0 + box_size && y < height && !occupied; y++)
            {
                for (int x = x0; x < x0 + box_size && x < width; x++)
                {
                    if (wall_point_occupied(table, x, y))
                    {
                        occupied = true;
                        break;
                    }
                }
            }
            if (occupied)
            {
                boxes++;
            }
        }
    }
    return boxes;
}

static double compute_fractal_dimension(MazeTable table)
{
    int max_dim = table.columns > table.rows ? table.columns * 2 + 1 : table.rows * 2 + 1;
    double sum_x = 0.0;
    double sum_y = 0.0;
    double sum_xx = 0.0;
    double sum_xy = 0.0;
    int n = 0;

    for (int box_size = 1; box_size <= max_dim; box_size *= 2)
    {
        int boxes = count_wall_boxes(table, box_size);
        if (boxes <= 0)
        {
            continue;
        }
        double x = log((double)max_dim / (double)box_size);
        double y = log((double)boxes);
        sum_x += x;
        sum_y += y;
        sum_xx += x * x;
        sum_xy += x * y;
        n++;
    }

    double denom = (double)n * sum_xx - sum_x * sum_x;
    if (n < 2 || fabs(denom) < 1e-12)
    {
        return 0.0;
    }
    return ((double)n * sum_xy - sum_x * sum_y) / denom;
}

static double compute_aspl_slow(MazeTable table)
{
    int total = table.rows * table.columns;
    if (total <= 1)
    {
        return 0.0;
    }

    double sum = 0.0;

    for (int start = 0; start < total; start++)
    {
        int *queue = (int *)malloc(sizeof(int) * total);
        int *dist = (int *)malloc(sizeof(int) * total);
        if (!queue || !dist)
        {
            free(queue);
            free(dist);
            return -1.0;
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

        for (int v = 0; v < total; v++)
        {
            if (v == start)
                continue;
            if (dist[v] < 0)
            {
                free(dist);
                free(queue);
                return -1.0;
            }
            sum += (double)dist[v];
        }

        free(dist);
        free(queue);
    }

    return sum / (double)(total * (total - 1));
}

static double compute_distance_entropy_slow(MazeTable table)
{
    int total = table.rows * table.columns;
    if (total <= 1)
    {
        return 0.0;
    }

    long long pairs = (long long)total * (long long)(total - 1);
    long long *hist = (long long *)calloc((size_t)total, sizeof(long long));
    if (!hist)
    {
        return -1.0;
    }

    for (int start = 0; start < total; start++)
    {
        int *queue = (int *)malloc(sizeof(int) * total);
        int *dist = (int *)malloc(sizeof(int) * total);
        if (!queue || !dist)
        {
            free(queue);
            free(dist);
            free(hist);
            return -1.0;
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

        for (int v = 0; v < total; v++)
        {
            if (v == start)
                continue;
            if (dist[v] < 0)
            {
                free(dist);
                free(queue);
                free(hist);
                return -1.0;
            }
            if (dist[v] < total)
            {
                hist[dist[v]]++;
            }
        }

        free(dist);
        free(queue);
    }

    double h = 0.0;
    const double inv_ln2 = 1.0 / log(2.0);
    for (int d = 1; d < total; d++)
    {
        if (hist[d] <= 0)
            continue;
        double p = (double)hist[d] / (double)pairs;
        h -= p * (log(p) * inv_ln2);
    }

    free(hist);
    return h;
}

static bool analyze_topology(MazeTable table, TopologyReport *out, int columns, int rows, int seed,
                             MazeGenerationMetrics metrics)
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
    int open_edges = count_open_edges(table);
    int components = count_components(table);

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
    out->steps_count = metrics.steps;
    out->memory_peak_bytes = metrics.peak_memory_bytes;
    out->wall_density = compute_wall_density(table);
    out->branch_ratio = (float)branch / (float)total;
    out->dead_end_count = dead_end;
    out->dead_end_ratio = (float)dead_end / (float)total;
    out->corridor_ratio = (float)corridor / (float)total;
    out->avg_degree = (float)degree_sum / (float)total;
    out->cycle_count = open_edges - total + components;
    out->diameter = diameter;
    out->longest_path = diameter;
    out->symmetry_score = compute_symmetry_score(table);
    out->fractal_dimension = compute_fractal_dimension(table);
    out->aspl = out->all_pairs_connected ? compute_aspl_slow(table) : -1.0;
    out->entropy = out->all_pairs_connected ? compute_distance_entropy_slow(table) : -1.0;
    return true;
}

/* --- Combined benchmark --- */

static bool run_case_combined(FILE *out, Generator generator, int columns, int rows, int seed, bool is_first)
{
    labirit_metrics_reset();
    MazeTable table = init_table(columns, rows, seed);
    if (!table.data)
    {
        return false;
    }

    labirit_metrics_reset_steps();
    long long started = now_microseconds();
    generator(table);
    long long finished = now_microseconds();
    MazeGenerationMetrics metrics = labirit_metrics_snapshot();

    TopologyReport report;
    if (!analyze_topology(table, &report, columns, rows, seed, metrics))
    {
        clear_table(&table);
        return false;
    }

    long long microseconds = finished - started;
    if (fprintf(out,
                "%s  {\"columns\":%d,\"rows\":%d,\"seed\":%d,\"microseconds\":%lld,"
                "\"all_pairs_connected\":%s,\"branch_ratio\":%.6f,"
                "\"steps_count\":%lld,\"memory_peak_bytes\":%zu,\"wall_density\":%.6f,"
                "\"dead_end_count\":%d,\"dead_end_ratio\":%.6f,\"corridor_ratio\":%.6f,"
                "\"avg_degree\":%.6f,\"cycle_count\":%d,\"diameter\":%d,"
                "\"longest_path\":%d,\"symmetry_score\":%.6f,\"fractal_dimension\":%.6f,"
                "\"aspl\":%.6f,\"entropy\":%.6f}",
                is_first ? "" : ",\n",
                report.columns, report.rows, report.seed, microseconds,
                report.all_pairs_connected ? "true" : "false",
                report.branch_ratio, report.steps_count, report.memory_peak_bytes,
                report.wall_density, report.dead_end_count, report.dead_end_ratio,
                report.corridor_ratio, report.avg_degree, report.cycle_count,
                report.diameter, report.longest_path, report.symmetry_score,
                report.fractal_dimension, report.aspl, report.entropy) < 0)
    {
        clear_table(&table);
        return false;
    }

    clear_table(&table);
    return true;
}

static bool benchmark_algorithm_combined(const char *path, Generator generator,
                                         const BenchmarkGridSize *sizes, size_t sizes_count,
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
            if (!run_case_combined(out, generator, sizes[i].columns, sizes[i].rows, seeds[s], first_entry))
            {
                fclose(out);
                return false;
            }
            first_entry = false;
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

static char maze_ascii_at(MazeTable table, int gx, int gy)
{
    int width = table.columns * 2 + 1;
    int height = table.rows * 2 + 1;
    if (gx == 0 || gy == 0 || gx == width - 1 || gy == height - 1)
    {
        return '#';
    }
    if ((gx % 2) == 1 && (gy % 2) == 1)
    {
        return ' ';
    }
    if ((gx % 2) == 0 && (gy % 2) == 0)
    {
        return '#';
    }
    if ((gx % 2) == 0)
    {
        int cell_x = gx / 2 - 1;
        int cell_y = gy / 2;
        return table.data[cell_y][cell_x].wall.right ? '#' : ' ';
    }

    int cell_x = gx / 2;
    int cell_y = gy / 2 - 1;
    return table.data[cell_y][cell_x].wall.bottom ? '#' : ' ';
}

static bool write_ascii_lines(FILE *out, MazeTable table)
{
    int width = table.columns * 2 + 1;
    int height = table.rows * 2 + 1;

    if (fprintf(out, "\"ascii\":[") < 0)
    {
        return false;
    }
    for (int gy = 0; gy < height; gy++)
    {
        if (fprintf(out, "%s\"", gy == 0 ? "" : ",") < 0)
        {
            return false;
        }
        for (int gx = 0; gx < width; gx++)
        {
            if (fputc(maze_ascii_at(table, gx, gy), out) == EOF)
            {
                return false;
            }
        }
        if (fputc('"', out) == EOF)
        {
            return false;
        }
    }
    return fprintf(out, "]") >= 0;
}

static bool write_example_maze(FILE *out, const char *algorithm, Generator generator, bool is_first)
{
    int columns = 24;
    int rows = 24;
    int seed = 101;

    labirit_metrics_reset();
    MazeTable table = init_table(columns, rows, seed);
    if (!table.data)
    {
        return false;
    }

    generator(table);

    bool ok = fprintf(out,
                      "%s  {\"algorithm\":\"%s\",\"columns\":%d,\"rows\":%d,\"seed\":%d,",
                      is_first ? "" : ",\n",
                      algorithm, columns, rows, seed) >= 0 &&
              write_ascii_lines(out, table) &&
              fprintf(out, "}") >= 0;

    clear_table(&table);
    return ok;
}

static bool write_example_mazes(void)
{
    if (!ensure_examples_dir())
    {
        return false;
    }

    FILE *out = fopen(RESULTS_DIR "/examples/maze_examples.json", "w");
    if (!out)
    {
        return false;
    }

    bool ok = fprintf(out, "[\n") >= 0 &&
              write_example_maze(out, "prim", prim_alg, true) &&
              write_example_maze(out, "growing_tree", growing_tree_alg, false) &&
              write_example_maze(out, "watson", watson_alg, false) &&
              write_example_maze(out, "dfs", dfs_topology_adapter, false) &&
              write_example_maze(out, "binary_tree", binary_algos, false) &&
              write_example_maze(out, "recursive_division", recursive_division_algorithm, false) &&
              fprintf(out, "\n]\n") >= 0;

    fclose(out);
    return ok;
}

bool run_combined_benchmarks(void)
{
    const int seeds[] = {101, 202, 303, 404, 505, 606, 707, 808, 909, 1010};

    if (!ensure_results_dir())
    {
        return false;
    }

    if (!benchmark_algorithm_combined(RESULTS_DIR "/prim.json", prim_alg,
                                      TIME_BENCH_CASES, sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                      seeds, sizeof(seeds) / sizeof(seeds[0])))
        return false;

    if (!benchmark_algorithm_combined(RESULTS_DIR "/growing_tree.json", growing_tree_alg,
                                      TIME_BENCH_CASES, sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                      seeds, sizeof(seeds) / sizeof(seeds[0])))
        return false;

    if (!benchmark_algorithm_combined(RESULTS_DIR "/watson.json", watson_alg,
                                      TIME_BENCH_CASES, sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                      seeds, sizeof(seeds) / sizeof(seeds[0])))
        return false;

    if (!benchmark_algorithm_combined(RESULTS_DIR "/dfs.json", dfs_topology_adapter,
                                      TIME_BENCH_CASES, sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                      seeds, sizeof(seeds) / sizeof(seeds[0])))
        return false;

    if (!benchmark_algorithm_combined(RESULTS_DIR "/binary_tree.json", binary_algos,
                                      TIME_BENCH_CASES, sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                      seeds, sizeof(seeds) / sizeof(seeds[0])))
        return false;

    if (!benchmark_algorithm_combined(RESULTS_DIR "/recursive_division.json", recursive_division_algorithm,
                                      TIME_BENCH_CASES, sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]),
                                      seeds, sizeof(seeds) / sizeof(seeds[0])))
        return false;

    if (!write_example_mazes())
        return false;

    return true;
}
