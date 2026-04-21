#include "../deter.h"

#define WATSON_IN_MAZE(cell) ((cell).wall.top)
#define WATSON_IN_WALK(cell) ((cell).wall.right)
#define WATSON_SET_IN_MAZE(cell) ((cell).wall.top = 1)
#define WATSON_SET_IN_WALK(cell) ((cell).wall.right = 1)
#define WATSON_CLR_IN_WALK(cell) ((cell).wall.right = 0)

static int watson_idx(TABLE table, int x, int y)
{
    return y * table.columns + x;
}

static void watson_xy(TABLE table, int index, int *x, int *y)
{
    *x = index % table.columns;
    *y = index / table.columns;
}

static void watson_remove_wall(TABLE table, int x1, int y1, int x2, int y2)
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

static bool watson_has_unvisited(TABLE table, Cell **state)
{
    for (int y = 0; y < table.rows; y++)
    {
        for (int x = 0; x < table.columns; x++)
        {
            if (!WATSON_IN_MAZE(state[y][x]))
            {
                return true;
            }
        }
    }
    return false;
}

static int watson_take_random_unvisited(TABLE table, Cell **state)
{
    int total = 0;
    for (int y = 0; y < table.rows; y++)
    {
        for (int x = 0; x < table.columns; x++)
        {
            if (!WATSON_IN_MAZE(state[y][x]))
            {
                total++;
            }
        }
    }

    if (total == 0)
    {
        return 0;
    }

    int target = rand() % total;
    int current = 0;

    for (int y = 0; y < table.rows; y++)
    {
        for (int x = 0; x < table.columns; x++)
        {
            if (!WATSON_IN_MAZE(state[y][x]))
            {
                if (current == target)
                {
                    return watson_idx(table, x, y);
                }
                current++;
            }
        }
    }

    return 0;
}

static int watson_random_neighbour(TABLE table, int x, int y)
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

void watson_alg(TABLE table)
{
    if (!table.data || table.rows <= 0 || table.columns <= 0)
    {
        return;
    }

    Cell **state = (Cell **)malloc(sizeof(Cell *) * table.rows);
    if (!state)
    {
        return;
    }

    for (int y = 0; y < table.rows; y++)
    {
        state[y] = (Cell *)calloc(table.columns, sizeof(Cell));
        if (!state[y])
        {
            for (int i = 0; i < y; i++)
            {
                free(state[i]);
            }
            free(state);
            return;
        }
    }

    int first_x = rand() % table.columns;
    int first_y = rand() % table.rows;
    WATSON_SET_IN_MAZE(state[first_y][first_x]);

    int max_path = table.rows * table.columns;
    int *path = (int *)malloc(sizeof(int) * max_path);
    if (!path)
    {
        for (int y = 0; y < table.rows; y++)
        {
            free(state[y]);
        }
        free(state);
        return;
    }

    while (watson_has_unvisited(table, state))
    {
        int start = watson_take_random_unvisited(table, state);
        int path_len = 1;
        path[0] = start;
        int start_x, start_y;
        watson_xy(table, start, &start_x, &start_y);
        WATSON_SET_IN_WALK(state[start_y][start_x]);

        while (1)
        {
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
            int x1, y1, x2, y2;
            watson_xy(table, path[i], &x1, &y1);
            watson_xy(table, path[i + 1], &x2, &y2);
            watson_remove_wall(table, x1, y1, x2, y2);
            WATSON_SET_IN_MAZE(state[y1][x1]);
        }
        for (int i = 0; i < path_len; i++)
        {
            int x, y;
            watson_xy(table, path[i], &x, &y);
            WATSON_SET_IN_MAZE(state[y][x]);
            WATSON_CLR_IN_WALK(state[y][x]);
        }
    }

    free(path);
    for (int y = 0; y < table.rows; y++)
    {
        free(state[y]);
    }
    free(state);
}
