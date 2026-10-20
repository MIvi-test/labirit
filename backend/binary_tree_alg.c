#include <stdlib.h>
#include "../deter.h"

_Static_assert(RAND_MAX == 2147483647, "Bit batching requires a 31-bit rand()");

void binary_algos(MazeTable table)
{
    LABIRIT_STEPS_DECLARE();
    if (!table.data || table.rows <= 0 || table.columns <= 0)
    {
        LABIRIT_STEPS_COMMIT();
        return;
    }

    unsigned int random_bits = 0;
    unsigned int bits_left = 0;

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

            bool go_right = can_right;
            if (can_right && can_bottom)
            {
                if (bits_left == 0)
                {
                    random_bits = (unsigned int)rand();
                    bits_left = 31;
                }
                go_right = (random_bits & 1u) == 0;
                random_bits >>= 1;
                bits_left--;
            }

            if (go_right)
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
    LABIRIT_STEPS_ADD((long long)table.rows * table.columns);
    LABIRIT_STEPS_COMMIT();
    return;
}
