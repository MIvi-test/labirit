#include "../deter.h"

MazeTable init_table(unsigned int columns, unsigned int rows, unsigned int seed)
{
    srand(seed);
    MazeTable table;
    table.columns = columns;
    table.rows = rows;
    table.seed = seed;
    table.data = NULL;

    Cell **data = (Cell **)malloc(rows * sizeof(Cell *));

    if (!data)
    {
        return table;
    }
    for (int j = 0; j < rows; j++)
    {
        Cell *p = (Cell *)malloc(columns * sizeof(Cell));
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
        for (int i = 0; i < columns; i++)
        {
            Cell obj;
            obj.all_bits = 0xFF;
            data[j][i] = obj;
        }
    }
    table.data = data;

    return table;
}

void clear_table(MazeTable *table)
{
    if (!table || !table->data)
    {
        return;
    }

    for (int j = 0; j < table->rows; j++)
    {
        free(table->data[j]);
    }
    free(table->data);
    table->data = NULL;
    return;
}
