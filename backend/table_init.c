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
    labirit_metrics_alloc(rows * sizeof(Cell *));
    for (unsigned int j = 0; j < rows; j++)
    {
        Cell *p = (Cell *)malloc(columns * sizeof(Cell));
        if (!p)
        {
            for (unsigned int er = 0; er < j; er++)
            {
                free(data[er]);
                labirit_metrics_free(columns * sizeof(Cell));
            }
            free(data);
            labirit_metrics_free(rows * sizeof(Cell *));
            return table;
        }
        labirit_metrics_alloc(columns * sizeof(Cell));
        data[j] = p;
        for (unsigned int i = 0; i < columns; i++)
        {
            labirit_metrics_step(1);
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
        labirit_metrics_free((size_t)table->columns * sizeof(Cell));
    }
    free(table->data);
    labirit_metrics_free((size_t)table->rows * sizeof(Cell *));
    table->data = NULL;
    return;
}
