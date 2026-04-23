#include "../deter.h"

bool dfs_algorithm(MazeTable table)
{
    if (!table.data || table.rows <= 0 || table.columns <= 0)
    {
        return false;
    }

    int total = table.rows * table.columns;
    bool *visited = (bool *)calloc((size_t)total, sizeof(bool));
    int *stack = (int *)malloc(sizeof(int) * (size_t)total);

    if (!visited || !stack)
    {
        free(visited);
        free(stack);
        return false;
    }

    int sp = 0;
    int start = 0;
    visited[start] = true;
    stack[sp++] = start;

    while (sp > 0)
    {
        int cur = stack[sp - 1];
        int x = cur % table.columns;
        int y = cur / table.columns;

        int neighbors[4];
        int count = 0;

        if (y > 0)
        {
            int next = (y - 1) * table.columns + x;
            if (!visited[next])
                neighbors[count++] = next;
        }
        if (x + 1 < table.columns)
        {
            int next = y * table.columns + (x + 1);
            if (!visited[next])
                neighbors[count++] = next;
        }
        if (y + 1 < table.rows)
        {
            int next = (y + 1) * table.columns + x;
            if (!visited[next])
                neighbors[count++] = next;
        }
        if (x > 0)
        {
            int next = y * table.columns + (x - 1);
            if (!visited[next])
                neighbors[count++] = next;
        }

        if (count == 0)
        {
            sp--;
            continue;
        }

        int next = neighbors[rand() % count];
        int nx = next % table.columns;
        int ny = next / table.columns;

        if (nx == x + 1 && ny == y)
        {
            table.data[y][x].wall.right = 0;
            table.data[y][x + 1].wall.left = 0;
        }
        else if (nx == x - 1 && ny == y)
        {
            table.data[y][x].wall.left = 0;
            table.data[y][x - 1].wall.right = 0;
        }
        else if (ny == y + 1 && nx == x)
        {
            table.data[y][x].wall.bottom = 0;
            table.data[y + 1][x].wall.top = 0;
        }
        else if (ny == y - 1 && nx == x)
        {
            table.data[y][x].wall.top = 0;
            table.data[y - 1][x].wall.bottom = 0;
        }

        visited[next] = true;
        stack[sp++] = next;
    }

    free(stack);
    free(visited);
    return true;
}
