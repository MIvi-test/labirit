#include <stdlib.h>
#include "deter.h"

void binary_algos(TABLE table)
{
    /*
    идём слева на право, убирается либо правая, либо верхняя стена
    Если индексы предельны, то выбора не даём
    благодаря индексации меняем у двух клеток значение
    Желаемая скорость алгоритма O(rows * columns)
    принимаем за факт, что пришла корректная таблица 
    */

    for(int j = 0; j < table.columns; j++)
    {
        for(int i = 0; i < table.rows; i++)
        {
            if(j == 0 && i == table.rows-1)
            {
                continue;
            }
            if(j == 0)
            {
                table.data[j][i].wall.right = 0;
                table.data[j][i+1].wall.left = 0;
            }
            else if(i == table.rows -1)
            {
                table.data[j][i].wall.top = 0;
                table.data[j-1][i].wall.bottom = 0;
            }
            else
            {
                if(rand()%2) // пусть это правая стена
                {
                    table.data[j][i].wall.right = 0;
                    table.data[j][i+1].wall.left = 0;
                }
                else
                {
                    table.data[j][i].wall.top = 0;
                    table.data[j-1][i].wall.bottom = 0;
                }
            }
        }
    }
    return;
}

