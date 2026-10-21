#include "../deter.h"
#include <limits.h>
#include <stdint.h>

#define WATSON_IN_MAZE(cell) ((cell).wall.top)
#define WATSON_IN_WALK(cell) ((cell).wall.right)
#define WATSON_SET_IN_MAZE(cell) ((cell).wall.top = 1)
#define WATSON_SET_IN_WALK(cell) ((cell).wall.right = 1)
#define WATSON_CLR_IN_WALK(cell) ((cell).wall.right = 0)

static int watson_idx(MazeTable table, int x, int y)
{
    return y * table.columns + x;
}

static void watson_xy(MazeTable table, int index, int *x, int *y)
{
    *x = index % table.columns;
    *y = index / table.columns;
}

static void watson_remove_wall(MazeTable table, int x1, int y1, int x2, int y2)
{
    if (x2 - x1 == 1)
    {
        table.data[y1][x1].wall.right = 0;
        table.data[y2][x2].wall.left = 0;
    }
    else if (x2 - x1 == -1)
    {
        table.data[y1][x1].wall.left = 0;
        table.data[y2][x2].wall.right = 0;
    }
    else if (y2 - y1 == 1)
    {
        table.data[y1][x1].wall.bottom = 0;
        table.data[y2][x2].wall.top = 0;
    }
    else if (y2 - y1 == -1)
    {
        table.data[y1][x1].wall.top = 0;
        table.data[y2][x2].wall.bottom = 0;
    }
}

static int watson_random_neighbour(MazeTable table, int x, int y)
{
    int variants[4];
    int count = 0;

    if (y > 0)
    {
        variants[count] = watson_idx(table, x, y - 1);
        count++;
    }
    if (x < table.columns - 1)
    {
        variants[count] = watson_idx(table, x + 1, y);
        count++;
    }
    if (y < table.rows - 1)
    {
        variants[count] = watson_idx(table, x, y + 1);
        count++;
    }
    if (x > 0)
    {
        variants[count] = watson_idx(table, x - 1, y);
        count++;
    }

    return variants[rand() % count];
}

void watson_alg(MazeTable table)
{
    LABIRIT_STEPS_DECLARE();
    if (!table.data || table.rows <= 0 || table.columns <= 0 ||
        table.rows > INT_MAX / table.columns ||
        (size_t)table.rows * table.columns > SIZE_MAX / sizeof(int))
    {
        LABIRIT_STEPS_COMMIT();
        return;
    }

    Cell **state = (Cell **)malloc(sizeof(Cell *) * table.rows);
    if (!state)
    {
        LABIRIT_STEPS_COMMIT();
        return;
    }
    labirit_metrics_alloc((size_t)table.rows * sizeof(Cell *));

    for (int y = 0; y < table.rows; y++)
    {
        state[y] = (Cell *)calloc(table.columns, sizeof(Cell));
        if (!state[y])
        {
            for (int i = 0; i < y; i++)
            {
                free(state[i]);
                labirit_metrics_free((size_t)table.columns * sizeof(Cell));
            }
            free(state);
            labirit_metrics_free((size_t)table.rows * sizeof(Cell *));
            LABIRIT_STEPS_COMMIT();
            return;
        }
        labirit_metrics_alloc((size_t)table.columns * sizeof(Cell));
    }

    int first_x = rand() % table.columns;
    int first_y = rand() % table.rows;
    WATSON_SET_IN_MAZE(state[first_y][first_x]);

    int max_path = table.rows * table.columns;
    int capacity = max_path < 64 ? max_path : 64;
    int *path = malloc(sizeof(int) * (size_t)capacity);
    if (!path)
    {
        for (int y = 0; y < table.rows; y++)
        {
            free(state[y]);
            labirit_metrics_free((size_t)table.columns * sizeof(Cell));
        }
        free(state);
        labirit_metrics_free((size_t)table.rows * sizeof(Cell *));
        LABIRIT_STEPS_COMMIT();
        return;
    }
    labirit_metrics_alloc((size_t)capacity * sizeof(int));

    int *unvisited = malloc((size_t)max_path * sizeof(int));
    int *positions = malloc((size_t)max_path * sizeof(int));
    if (!unvisited || !positions)
    {
        free(unvisited);
        free(positions);
        unvisited = positions = NULL;
        goto cleanup;
    }
    labirit_metrics_alloc((size_t)max_path * sizeof(int));
    labirit_metrics_alloc((size_t)max_path * sizeof(int));
    int remaining = 0;
    int first = watson_idx(table, first_x, first_y);
    for (int index = 0; index < max_path; index++)
    {
        if (index != first)
        {
            positions[index] = remaining;
            unvisited[remaining++] = index;
        }
    }

    while (remaining)
    {
        LABIRIT_STEPS_ADD(1);
        int start = unvisited[rand() % remaining];
        int path_len = 1;
        path[0] = start;
        int start_x, start_y;
        watson_xy(table, start, &start_x, &start_y);
        WATSON_SET_IN_WALK(state[start_y][start_x]);

        while (1)
        {
            LABIRIT_STEPS_ADD(1);
            int cur_x, cur_y;
            watson_xy(table, path[path_len - 1], &cur_x, &cur_y);
            if (WATSON_IN_MAZE(state[cur_y][cur_x]))
            {
                break;
            }

            int next = watson_random_neighbour(table, cur_x, cur_y);
            int next_x, next_y;
            watson_xy(table, next, &next_x, &next_y);

            if (WATSON_IN_WALK(state[next_y][next_x]))
            {
                while (path_len > 0)
                {
                    LABIRIT_STEPS_ADD(1);
                    int tail_x, tail_y;
                    watson_xy(table, path[path_len - 1], &tail_x, &tail_y);
                    if (path[path_len - 1] == next)
                    {
                        break;
                    }
                    WATSON_CLR_IN_WALK(state[tail_y][tail_x]);
                    path_len--;
                }
            }
            else
            {
                if (path_len < max_path)
                {
                    if (path_len == capacity)
                    {
                        int grown = capacity > max_path / 2 ? max_path : capacity * 2;
                        int *replacement = realloc(path, (size_t)grown * sizeof(int));
                        if (!replacement)
                            goto cleanup;
                        /* Conservatively account for both blocks during a moving realloc. */
                        labirit_metrics_alloc((size_t)grown * sizeof(int));
                        labirit_metrics_free((size_t)capacity * sizeof(int));
                        path = replacement;
                        capacity = grown;
                    }
                    path[path_len] = next;
                    WATSON_SET_IN_WALK(state[next_y][next_x]);
                    path_len++;
                }
                else
                {
                    break;
                }
            }
        }

        for (int i = 0; i < path_len - 1; i++)
        {
            LABIRIT_STEPS_ADD(1);
            int x1, y1, x2, y2;
            watson_xy(table, path[i], &x1, &y1);
            watson_xy(table, path[i + 1], &x2, &y2);
            watson_remove_wall(table, x1, y1, x2, y2);

        }
        for (int i = 0; i < path_len; i++)
        {
            LABIRIT_STEPS_ADD(1);
            int x, y;
            watson_xy(table, path[i], &x, &y);
            if (!WATSON_IN_MAZE(state[y][x]))
            {
                int position = positions[path[i]];
                int last = unvisited[--remaining];
                unvisited[position] = last;
                positions[last] = position;
                WATSON_SET_IN_MAZE(state[y][x]);
            }
            WATSON_CLR_IN_WALK(state[y][x]);
        }
    }

cleanup:
    if (unvisited)
    {
        free(unvisited);
        free(positions);
        labirit_metrics_free((size_t)max_path * sizeof(int));
        labirit_metrics_free((size_t)max_path * sizeof(int));
    }
    free(path);
    labirit_metrics_free((size_t)capacity * sizeof(int));
    for (int y = 0; y < table.rows; y++)
    {
        free(state[y]);
        labirit_metrics_free((size_t)table.columns * sizeof(Cell));
    }
    free(state);
    labirit_metrics_free((size_t)table.rows * sizeof(Cell *));
    LABIRIT_STEPS_COMMIT();
}
