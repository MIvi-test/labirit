#include "../deter.h"
#include <limits.h>
#include <stdint.h>

MazeTable init_table(unsigned int columns, unsigned int rows, unsigned int seed)
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
    memset(cells, 0xFF, count * sizeof(Cell));
    labirit_metrics_alloc(bytes);
    LABIRIT_STEPS_ADD(count);
    table.data = data;
    LABIRIT_STEPS_COMMIT();
    return table;
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
