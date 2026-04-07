#include "../deter.h"

void reqursive_alg(TABLE table)
{
    if (table.columns <= 1)
    {
        return;
    }
    if (table.rows <= 1)
    {
        return;
    }
    if (rand() % 2)
    {
        int x = rand() % (table.columns - 1);
        for (int j = 0; j < table.columns; j++)
        {
            if (table.data[j][x].wall.right == 1)
            {
                table.data[j][x].wall.right = 0;
            }
            if (table.data[j][x + 1].wall.right == 1)
            {
                table.data[j][x + 1].wall.left = 0;
            }
        }
        // пусть это вертикаль, axis 1
    }
    else
    {
        int y = rand() % (table.rows - 1);
        for (int i = 0; i < table.rows; i++)
        {
            if (table.data[y][i].wall.bottom == 1)
            {
                table.data[y][i].wall.bottom = 0;
            }
            if (table.data[y + 1][i].wall.top == 1)
            {
                table.data[y + 1][i].wall.top = 0;
            }
        }
        // пусть это горизонталь, axis 0
    }
    TABLE new_table = table;
    if (rand() & 2)
    {
        new_table.columns /= 2;
        reqursive_alg(new_table);
        new_table.data + new_table.columns;
        reqursive_alg(new_table);
    }
    else
    {
        new_table.rows /= 2;
        reqursive_alg(new_table);
        new_table.data + new_table.rows;
        reqursive_alg(new_table);
    }
    return;
}
