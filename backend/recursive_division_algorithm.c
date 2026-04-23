#include "../deter.h"

static void open_all_internal_walls(MazeTable table)
{
    for (int y = 0; y < table.rows; y++)
    {
        for (int x = 0; x < table.columns; x++)
        {
            if (x + 1 < table.columns)
            {
                table.data[y][x].wall.right = 0;
                table.data[y][x + 1].wall.left = 0;
            }
            if (y + 1 < table.rows)
            {
                table.data[y][x].wall.bottom = 0;
                table.data[y + 1][x].wall.top = 0;
            }
        }
    }
}

static void divide(int x, int y, int w, int h, MazeTable table)
{
    if (w <= 1 || h <= 1)
    {
        return;
    }

    bool horizontal = false;
    if (w < h)
        horizontal = true;
    else if (w > h)
        horizontal = false;
    else
        horizontal = (rand() % 2) == 0;

    if (horizontal)
    {
        int wall_y = y + (rand() % (h - 1));      /* wall between wall_y and wall_y+1 */
        int passage_x = x + (rand() % w);         /* one gap */

        for (int i = 0; i < w; i++)
        {
            int cx = x + i;
            if (cx == passage_x)
            {
                continue;
            }
            table.data[wall_y][cx].wall.bottom = 1;
            table.data[wall_y + 1][cx].wall.top = 1;
        }

        divide(x, y, w, (wall_y - y + 1), table);
        divide(x, wall_y + 1, w, (y + h - (wall_y + 1)), table);
    }
    else
    {
        int wall_x = x + (rand() % (w - 1));      /* wall between wall_x and wall_x+1 */
        int passage_y = y + (rand() % h);         /* one gap */

        for (int j = 0; j < h; j++)
        {
            int cy = y + j;
            if (cy == passage_y)
            {
                continue;
            }
            table.data[cy][wall_x].wall.right = 1;
            table.data[cy][wall_x + 1].wall.left = 1;
        }

        divide(x, y, (wall_x - x + 1), h, table);
        divide(wall_x + 1, y, (x + w - (wall_x + 1)), h, table);
    }
}

void recursive_division_algorithm(MazeTable table)
{
    if (!table.data || table.columns <= 0 || table.rows <= 0)
    {
        return;
    }
    if (table.columns == 1 && table.rows == 1)
    {
        return;
    }

    /* Recursive division traditionally adds walls to an empty grid.
       Here we first open all internal walls, then add dividing walls. */
    open_all_internal_walls(table);
    divide(0, 0, table.columns, table.rows, table);
    return;
}
