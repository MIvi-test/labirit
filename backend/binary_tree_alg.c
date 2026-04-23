#include <stdlib.h>
#include "../deter.h"

void binary_algos(MazeTable table)
{
    if (!table.data || table.rows <= 0 || table.columns <= 0)
    {
        return;
    }

    for (int y = 0; y < table.rows; y++)
    {
        for (int x = 0; x < table.columns; x++)
        {
            if (x == table.columns - 1 && y == table.rows - 1)
            {
                continue;
            }

            bool can_right = (x + 1 < table.columns);
            bool can_bottom = (y + 1 < table.rows);

            if (can_right && (!can_bottom || (rand() % 2 == 0)))
            {
                table.data[y][x].wall.right = 0;
                table.data[y][x + 1].wall.left = 0;
            }
            else if (can_bottom)
            {
                table.data[y][x].wall.bottom = 0;
                table.data[y + 1][x].wall.top = 0;
            }
        }
    }
    return;
}
