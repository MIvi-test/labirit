#include "../deter.h"
#include <limits.h>
#include <stdint.h>

static MazeTable init_table_mode(unsigned int columns, unsigned int rows, unsigned int seed, bool open)
{
    LABIRIT_STEPS_DECLARE();
    srand(seed);
    MazeTable table = {.columns = columns <= INT_MAX ? (int)columns : 0,
                       .rows = rows <= INT_MAX ? (int)rows : 0,
                       .seed = (int)seed, .data = NULL};
    if (!columns || !rows || columns > INT_MAX || rows > INT_MAX ||
        rows > INT_MAX / columns || sizeof(Cell *) > SIZE_MAX / rows)
        return table;
    size_t count = (size_t)columns * rows;
    size_t pointers = (size_t)rows * sizeof(Cell *);
    if (count > (SIZE_MAX - pointers) / sizeof(Cell))
        return table;
    size_t bytes = pointers + count * sizeof(Cell);
    Cell **data = malloc(bytes);
    if (!data)
        return table;
    /* This allocation owns both the row pointers and all cells. */
    Cell *cells = (Cell *)((unsigned char *)data + pointers);
    for (unsigned int y = 0; y < rows; y++)
        data[y] = cells + (size_t)y * columns;
    Cell fill = {.all_bits = 0xFF};
    if (open)
    {
        fill.wall.top = fill.wall.right = fill.wall.bottom = fill.wall.left = 0;
    }
    memset(cells, fill.all_bits, count * sizeof(Cell));
    if (open)
    {
        for (unsigned int x = 0; x < columns; x++)
        {
            data[0][x].wall.top = 1;
            data[rows - 1][x].wall.bottom = 1;
        }
        for (unsigned int y = 0; y < rows; y++)
        {
            data[y][0].wall.left = 1;
            data[y][columns - 1].wall.right = 1;
        }
    }
    labirit_metrics_alloc(bytes);
    LABIRIT_STEPS_ADD(count);
    table.data = data;
    LABIRIT_STEPS_COMMIT();
    return table;
}

MazeTable init_table(unsigned int columns, unsigned int rows, unsigned int seed)
{
    return init_table_mode(columns, rows, seed, false);
}

MazeTable init_table_open(unsigned int columns, unsigned int rows, unsigned int seed)
{
    return init_table_mode(columns, rows, seed, true);
}

void clear_table(MazeTable *table)
{
    if (!table || !table->data)
        return;
    size_t bytes = (size_t)table->rows * sizeof(Cell *) +
                   (size_t)table->rows * table->columns * sizeof(Cell);
    free(table->data);
    labirit_metrics_free(bytes);
    table->data = NULL;
}
