#include "../deter.h"

static void growing_tree_remove_wall(MazeTable table, int x1, int y1, int x2, int y2)
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

void growing_tree_alg(MazeTable table)
{
    if (!table.data || table.rows <= 0 || table.columns <= 0)
    {
        return;
    }

    int total = table.rows * table.columns;
    bool *visited = (bool *)calloc(total, sizeof(bool));
    int *active = (int *)malloc(sizeof(int) * total);
    int *active_age = (int *)malloc(sizeof(int) * total);

    if (!visited || !active || !active_age)
    {
        free(visited);
        free(active);
        free(active_age);
        return;
    }

    int start_x = rand() % table.columns;
    int start_y = rand() % table.rows;
    int start = start_y * table.columns + start_x;

    visited[start] = true;
    active[0] = start;
    active_age[0] = 0;
    int active_count = 1;
    int tick = 1;

    while (active_count > 0)
    {
        int chosen_pos = 0;
        if (rand() % 100 < 60)
        {
            int best_age = active_age[0];
            for (int i = 1; i < active_count; i++)
            {
                if (active_age[i] > best_age)
                {
                    best_age = active_age[i];
                    chosen_pos = i;
                }
            }
        }
        else
        {
            chosen_pos = rand() % active_count;
        }

        int current = active[chosen_pos];
        int x = current % table.columns;
        int y = current / table.columns;

        int neighbors[4];
        int count = 0;

        if (y > 0)
        {
            int next = (y - 1) * table.columns + x;
            if (!visited[next])
            {
                neighbors[count++] = next;
            }
        }
        if (x < table.columns - 1)
        {
            int next = y * table.columns + (x + 1);
            if (!visited[next])
            {
                neighbors[count++] = next;
            }
        }
        if (y < table.rows - 1)
        {
            int next = (y + 1) * table.columns + x;
            if (!visited[next])
            {
                neighbors[count++] = next;
            }
        }
        if (x > 0)
        {
            int next = y * table.columns + (x - 1);
            if (!visited[next])
            {
                neighbors[count++] = next;
            }
        }

        if (count == 0)
        {
            active[chosen_pos] = active[active_count - 1];
            active_age[chosen_pos] = active_age[active_count - 1];
            active_count--;
            continue;
        }

        int next = neighbors[rand() % count];
        int next_x = next % table.columns;
        int next_y = next / table.columns;

        growing_tree_remove_wall(table, x, y, next_x, next_y);
        visited[next] = true;
        active[active_count++] = next;
        active_age[active_count - 1] = tick++;
    }

    free(active_age);
    free(active);
    free(visited);
}
