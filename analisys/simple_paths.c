#include "simple_paths.h"

static inline int in_bounds(MazeTable t, int x, int y)
{
    return x >= 0 && y >= 0 && x < t.columns && y < t.rows;
}

static inline int idx(MazeTable t, int x, int y)
{
    return y * t.columns + x;
}

static int collect_neighbors(MazeTable t, int x, int y, int out[4])
{
    int n = 0;
    if (t.data[y][x].wall.top == 0 && y > 0)
        out[n++] = idx(t, x, y - 1);
    if (t.data[y][x].wall.right == 0 && x < t.columns - 1)
        out[n++] = idx(t, x + 1, y);
    if (t.data[y][x].wall.bottom == 0 && y < t.rows - 1)
        out[n++] = idx(t, x, y + 1);
    if (t.data[y][x].wall.left == 0 && x > 0)
        out[n++] = idx(t, x - 1, y);
    return n;
}

static uint64_t dfs_count(MazeTable t,
                          int cur,
                          int target,
                          unsigned char *visited,
                          int steps,
                          int max_len,
                          uint64_t cap,
                          uint64_t so_far)
{
    if (cap != 0 && so_far >= cap)
        return cap;

    if (cur == target)
        return so_far + 1;

    if (max_len >= 0 && steps >= max_len)
        return so_far;

    int x = cur % t.columns;
    int y = cur / t.columns;

    int nbrs[4];
    int n = collect_neighbors(t, x, y, nbrs);

    for (int i = 0; i < n; i++)
    {
        int next = nbrs[i];
        if (visited[next])
            continue;

        visited[next] = 1;
        so_far = dfs_count(t, next, target, visited, steps + 1, max_len, cap, so_far);
        visited[next] = 0;

        if (cap != 0 && so_far >= cap)
            return cap;
    }

    return so_far;
}

uint64_t count_simple_paths_slow(MazeTable table,
                                int sx, int sy,
                                int tx, int ty,
                                int max_len,
                                uint64_t cap)
{
    if (!table.data || table.rows <= 0 || table.columns <= 0)
        return 0;
    if (!in_bounds(table, sx, sy) || !in_bounds(table, tx, ty))
        return 0;

    int total = table.rows * table.columns;
    unsigned char *visited = (unsigned char *)calloc((size_t)total, 1);
    if (!visited)
        return 0;

    int start = idx(table, sx, sy);
    int target = idx(table, tx, ty);

    visited[start] = 1;
    uint64_t ans = dfs_count(table, start, target, visited, 0, max_len, cap, 0);
    free(visited);
    return ans;
}

