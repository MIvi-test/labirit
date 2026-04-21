#include <stdlib.h>
#include "../deter.h"

void binary_algos(TABLE table)
{

    for (int axis = 0; axis < 2; axis++)
    {
        for (int x = 0; x < table.rows; x++)
        {
            for (int y = 0; y < table.columns; y++)
            {
                if (x == 0 && y == table.columns - 1)
                {
                    continue;
                }
                if (axis)
                {
                    if (rand() % 2)
                    {
                        if (y == 0)
                        {
                            continue;
                        }
                        table.data[y][x].wall.top = 0;
                        table.data[y + 1][x].wall.bottom = 0;
                    }
                }
                else
                {
                    if (rand() % 2)
                    {
                        if (x == table.rows - 1)
                        {
                            continue;
                        }
                        table.data[y][x].wall.right = 0;
                        table.data[y][x + 1].wall.left = 0;
                    }
                }
            }
        }
    }
    return;
}
