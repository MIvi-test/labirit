#include "../deter.h"

TABLE INIT_TABLE(unsigned int columns, unsigned int rows, unsigned int seed)
{
    /*Инициализация массива, создается указатель на массив указателей(стобцов)
    цикл инициализации массивов клеток, размер одного массива = количество строк
    все значение обязательно неотрицательны, результат в указателе TABLE
    */
    srand(seed);
    TABLE table;
    table.columns = columns;
    table.rows = rows;
    table.seed = seed;
    table.data = NULL;
    Cell **data = (Cell **)malloc(columns * sizeof(Cell *));
    if (!table)
    {
        return table;
    }
    for (int j = 0; j < columns; j++)
    {
        Cell *p = (Cell *)malloc(rows * sizeof(Cell));
        if (!p)
        {
            for (int er = j - 1; er >= 0; er--)
            {
                free(data[er]);
            }
            free(data);
            return table;
        }
        data[j] = p;
        for (int i = 0; i < rows; i++)
        {
            Cell obj;
            obj.all_bits = 0xFF; // 0b11111111
            data[j][i] = obj;
        }
    }
    table.data = data;

    return table;
}

void CLEAR_TABLE(TABLE *table)
{
    for(int j = 0; j < (*table)->columns; j++)
    {
        free((*table).data[j]);
    }
    free((*table).data);
    (*table).data = NULL;
    return;
}
