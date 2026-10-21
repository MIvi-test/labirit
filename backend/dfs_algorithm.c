#include "../deter.h"
#include <limits.h>
#include <stdint.h>

typedef struct
{
    int x, y;
} DfsFrame;

static void mark_visited(unsigned char *remaining, int x, int y, MazeTable table)
{
    size_t index = (size_t)y * table.columns + x;
    if (y > 0) remaining[index - table.columns] &= ~4;
    if (x + 1 < table.columns) remaining[index + 1] &= ~8;
    if (y + 1 < table.rows) remaining[index + table.columns] &= ~1;
    if (x > 0) remaining[index - 1] &= ~2;
}

bool dfs_algorithm(MazeTable table)
{
    LABIRIT_STEPS_DECLARE();
    if (!table.data || table.rows <= 0 || table.columns <= 0 ||
        table.rows > INT_MAX / table.columns)
        return false;
    size_t total = (size_t)table.rows * table.columns;
    if (total > SIZE_MAX / sizeof(DfsFrame))
        return false;
    unsigned char *remaining = malloc(total);
    DfsFrame *stack = malloc(total * sizeof(DfsFrame));
    if (!remaining || !stack)
    {
        free(remaining);
        free(stack);
        return false;
    }
    labirit_metrics_alloc(total);
    labirit_metrics_alloc(total * sizeof(DfsFrame));
    size_t sp = 1;
    for (int y = 0; y < table.rows; y++)
        for (int x = 0; x < table.columns; x++)
            remaining[(size_t)y * table.columns + x] =
                (y > 0 ? 1 : 0) | (x + 1 < table.columns ? 2 : 0) |
                (y + 1 < table.rows ? 4 : 0) | (x > 0 ? 8 : 0);
    mark_visited(remaining, 0, 0, table);
    stack[0] = (DfsFrame){0, 0};
    static const unsigned char counts[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
    while (sp)
    {
        LABIRIT_STEPS_ADD(1);
        DfsFrame *frame = &stack[sp - 1];
        int x = frame->x, y = frame->y;
        size_t index = (size_t)y * table.columns + x;
        unsigned char mask = remaining[index];
        if (!mask)
        {
            sp--;
            continue;
        }
        static const unsigned char choices[16][4] = {
            {0}, {1}, {2}, {1,2}, {4}, {1,4}, {2,4}, {1,2,4},
            {8}, {1,8}, {2,8}, {1,2,8}, {4,8}, {1,4,8}, {2,4,8}, {1,2,4,8}
        };
        unsigned char direction = choices[mask][rand() % counts[mask]];
        int nx = x, ny = y;
        switch (direction)
        {
        case 1:
            ny--; table.data[y][x].wall.top = 0;
            table.data[ny][nx].wall.bottom = 0; break;
        case 2:
            nx++; table.data[y][x].wall.right = 0;
            table.data[ny][nx].wall.left = 0; break;
        case 4:
            ny++; table.data[y][x].wall.bottom = 0;
            table.data[ny][nx].wall.top = 0; break;
        default:
            nx--; table.data[y][x].wall.left = 0;
            table.data[ny][nx].wall.right = 0; break;
        }
        mark_visited(remaining, nx, ny, table);
        LABIRIT_STEPS_ADD(4);
        stack[sp++] = (DfsFrame){nx, ny};
    }
    free(stack);
    labirit_metrics_free(total * sizeof(DfsFrame));
    free(remaining);
    labirit_metrics_free(total);
    LABIRIT_STEPS_COMMIT();
    return true;
}
