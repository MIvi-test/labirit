#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../backend/all_algorithms.h"

typedef void (*GenFn)(MazeTable);

static void gen_dfs_adapter(MazeTable t)
{
    (void)dfs_algorithm(t);
}

typedef struct
{
    const char *name;
    GenFn fn;
} Algorithm;

static bool is_open_between(MazeTable t, int x1, int y1, int x2, int y2)
{
    if (x2 == x1 + 1 && y2 == y1)
    {
        return t.data[y1][x1].wall.right == 0 && t.data[y2][x2].wall.left == 0;
    }
    if (x2 == x1 - 1 && y2 == y1)
    {
        return t.data[y1][x1].wall.left == 0 && t.data[y2][x2].wall.right == 0;
    }
    if (y2 == y1 + 1 && x2 == x1)
    {
        return t.data[y1][x1].wall.bottom == 0 && t.data[y2][x2].wall.top == 0;
    }
    if (y2 == y1 - 1 && x2 == x1)
    {
        return t.data[y1][x1].wall.top == 0 && t.data[y2][x2].wall.bottom == 0;
    }
    return false;
}

static bool check_boundary_closed(const char *alg, MazeTable t)
{
    for (int x = 0; x < t.columns; x++)
    {
        if (t.data[0][x].wall.top == 0)
        {
            fprintf(stderr, "[%s] boundary open: top at (x=%d,y=0)\n", alg, x);
            return false;
        }
        if (t.data[t.rows - 1][x].wall.bottom == 0)
        {
            fprintf(stderr, "[%s] boundary open: bottom at (x=%d,y=%d)\n", alg, x, t.rows - 1);
            return false;
        }
    }
    for (int y = 0; y < t.rows; y++)
    {
        if (t.data[y][0].wall.left == 0)
        {
            fprintf(stderr, "[%s] boundary open: left at (x=0,y=%d)\n", alg, y);
            return false;
        }
        if (t.data[y][t.columns - 1].wall.right == 0)
        {
            fprintf(stderr, "[%s] boundary open: right at (x=%d,y=%d)\n", alg, t.columns - 1, y);
            return false;
        }
    }
    return true;
}

static bool check_symmetry(const char *alg, MazeTable t)
{
    for (int y = 0; y < t.rows; y++)
    {
        for (int x = 0; x < t.columns; x++)
        {
            if (x + 1 < t.columns)
            {
                int a = t.data[y][x].wall.right;
                int b = t.data[y][x + 1].wall.left;
                if (a != b)
                {
                    fprintf(stderr, "[%s] wall mismatch (x=%d,y=%d) right=%d vs neighbor left=%d\n",
                            alg, x, y, a, b);
                    return false;
                }
            }
            if (y + 1 < t.rows)
            {
                int a = t.data[y][x].wall.bottom;
                int b = t.data[y + 1][x].wall.top;
                if (a != b)
                {
                    fprintf(stderr, "[%s] wall mismatch (x=%d,y=%d) bottom=%d vs neighbor top=%d\n",
                            alg, x, y, a, b);
                    return false;
                }
            }
        }
    }
    return true;
}

static bool check_connected(const char *alg, MazeTable t)
{
    int total = t.rows * t.columns;
    bool *vis = (bool *)calloc((size_t)total, sizeof(bool));
    int *q = (int *)malloc(sizeof(int) * (size_t)total);
    if (!vis || !q)
    {
        free(vis);
        free(q);
        fprintf(stderr, "[%s] OOM in connectivity check\n", alg);
        return false;
    }

    int head = 0, tail = 0;
    vis[0] = true;
    q[tail++] = 0;

    while (head < tail)
    {
        int cur = q[head++];
        int x = cur % t.columns;
        int y = cur / t.columns;

        if (y > 0 && is_open_between(t, x, y, x, y - 1))
        {
            int next = (y - 1) * t.columns + x;
            if (!vis[next])
            {
                vis[next] = true;
                q[tail++] = next;
            }
        }
        if (x + 1 < t.columns && is_open_between(t, x, y, x + 1, y))
        {
            int next = y * t.columns + (x + 1);
            if (!vis[next])
            {
                vis[next] = true;
                q[tail++] = next;
            }
        }
        if (y + 1 < t.rows && is_open_between(t, x, y, x, y + 1))
        {
            int next = (y + 1) * t.columns + x;
            if (!vis[next])
            {
                vis[next] = true;
                q[tail++] = next;
            }
        }
        if (x > 0 && is_open_between(t, x, y, x - 1, y))
        {
            int next = y * t.columns + (x - 1);
            if (!vis[next])
            {
                vis[next] = true;
                q[tail++] = next;
            }
        }
    }

    bool ok = true;
    for (int i = 0; i < total; i++)
    {
        if (!vis[i])
        {
            fprintf(stderr, "[%s] disconnected cell index=%d\n", alg, i);
            ok = false;
            break;
        }
    }

    free(q);
    free(vis);
    return ok;
}

static bool run_one(const Algorithm *alg, int cols, int rows, int seed)
{
    MazeTable t = init_table((unsigned int)cols, (unsigned int)rows, (unsigned int)seed);
    if (!t.data)
    {
        fprintf(stderr, "[%s] init_table failed cols=%d rows=%d seed=%d\n", alg->name, cols, rows, seed);
        return false;
    }

    alg->fn(t);

    bool ok = true;
    ok = check_symmetry(alg->name, t) && ok;
    ok = check_boundary_closed(alg->name, t) && ok;
    ok = check_connected(alg->name, t) && ok;

    clear_table(&t);
    return ok;
}

static const char *get_arg_value(int argc, char **argv, const char *key)
{
    for (int i = 1; i + 1 < argc; i++)
    {
        if (strcmp(argv[i], key) == 0)
        {
            return argv[i + 1];
        }
    }
    return NULL;
}

static bool has_flag(int argc, char **argv, const char *flag)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], flag) == 0)
        {
            return true;
        }
    }
    return false;
}

int main(int argc, char **argv)
{
    const Algorithm algs[] = {
        {"prim", prim_alg},
        {"growing_tree", growing_tree_alg},
        {"watson", watson_alg},
        {"dfs", gen_dfs_adapter},
        {"binary_tree", binary_algos},
        {"recursive_division", recursive_division_algorithm},
    };

    const int sizes[][2] = {{2, 2}, {3, 3}, {4, 4}, {8, 8}, {16, 12}};
    const int seeds[] = {1, 2, 3, 101, 202};

    const char *only_alg = get_arg_value(argc, argv, "--alg");
    bool verbose = has_flag(argc, argv, "--verbose");

    int failures = 0;
    int total = 0;

    for (size_t a = 0; a < sizeof(algs) / sizeof(algs[0]); a++)
    {
        if (only_alg && strcmp(only_alg, algs[a].name) != 0)
        {
            continue;
        }
        for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); s++)
        {
            for (size_t k = 0; k < sizeof(seeds) / sizeof(seeds[0]); k++)
            {
                total++;
                int cols = sizes[s][0];
                int rows = sizes[s][1];
                int seed = seeds[k];
                if (verbose)
                {
                    printf("RUN %s cols=%d rows=%d seed=%d\n", algs[a].name, cols, rows, seed);
                    fflush(stdout);
                }
                if (!run_one(&algs[a], cols, rows, seed))
                {
                    failures++;
                }
            }
        }
    }

    if (failures == 0)
    {
        printf("OK: %d/%d tests passed\n", total, total);
        return 0;
    }

    fprintf(stderr, "FAIL: %d/%d tests failed\n", failures, total);
    return 1;
}

