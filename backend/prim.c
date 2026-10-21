#include "../deter.h"
#include <limits.h>
#include <stdint.h>

typedef struct massive
{
    int x1;
    int y1;
    int x2;
    int y2;
} massive;

typedef struct
{
    massive *edges;
    size_t count;
    size_t capacity;
} Frontier;

static unsigned char *visited;
static int visited_columns;
static size_t visited_bytes;

static void destroy_Bind(Frontier *frontier)
{
    free(frontier->edges);
    labirit_metrics_free(frontier->capacity * sizeof(massive));
}

static bool add_bind(Frontier *frontier, int x1, int y1, int x2, int y2)
{
    if (frontier->count == frontier->capacity)
    {
        size_t capacity = frontier->capacity ? frontier->capacity * 2 : 64;
        if (capacity < frontier->capacity || capacity > SIZE_MAX / sizeof(massive))
            return false;
        massive *edges = realloc(frontier->edges, capacity * sizeof(massive));
        if (!edges)
            return false;
        labirit_metrics_alloc(capacity * sizeof(massive));
        labirit_metrics_free(frontier->capacity * sizeof(massive));
        frontier->edges = edges;
        frontier->capacity = capacity;
    }
    frontier->edges[frontier->count++] = (massive){x1, y1, x2, y2};
    return true;
}

static massive pop_bind(Frontier *frontier)
{
    size_t index = (size_t)rand() % frontier->count;
    massive edge = frontier->edges[index];
    frontier->edges[index] = frontier->edges[--frontier->count];
    return edge;
}

static bool add_visit(int x, int y)
{
    visited[(size_t)y * visited_columns + x] = 1;
    return true;
}

static void destroy_Visited(void)
{
    free(visited);
    labirit_metrics_free(visited_bytes);
    visited = NULL;
    visited_bytes = 0;
}

static bool in_visited(int x, int y)
{
    return visited[(size_t)y * visited_columns + x] != 0;
}

void remove_wall(MazeTable table, massive bind)
{
    if (bind.x2 - bind.x1 == 1)
    {
        table.data[bind.y1][bind.x1].wall.right = 0;
        table.data[bind.y2][bind.x2].wall.left = 0;
    }
    else if (bind.x2 - bind.x1 == -1)
    {
        table.data[bind.y1][bind.x1].wall.left = 0;
        table.data[bind.y2][bind.x2].wall.right = 0;
    }
    else if (bind.y2 - bind.y1 == 1)
    {
        table.data[bind.y1][bind.x1].wall.bottom = 0;
        table.data[bind.y2][bind.x2].wall.top = 0;
    }
    else if (bind.y2 - bind.y1 == -1)
    {
        table.data[bind.y1][bind.x1].wall.top = 0;
        table.data[bind.y2][bind.x2].wall.bottom = 0;
    }
}

void prim_alg(MazeTable table)
{
    LABIRIT_STEPS_DECLARE();
    Frontier frontier = {0};
    destroy_Visited();

    if (!table.data || table.rows <= 0 || table.columns <= 0 ||
        table.rows > INT_MAX / table.columns)
    {
        LABIRIT_STEPS_COMMIT();
        return;
    }
    visited_columns = table.columns;
    visited_bytes = (size_t)table.rows * table.columns;
    visited = calloc(visited_bytes, 1);
    if (!visited)
    {
        visited_bytes = 0;
        LABIRIT_STEPS_COMMIT();
        return;
    }
    labirit_metrics_alloc(visited_bytes);
    add_visit(0, 0);
    if (table.rows > 1 && !add_bind(&frontier, 0, 0, 0, 1))
        goto cleanup;
    if (table.columns > 1 && !add_bind(&frontier, 0, 0, 1, 0))
        goto cleanup;
    while (frontier.count)
    {
        LABIRIT_STEPS_ADD(1);
        massive bind = pop_bind(&frontier);
        if (in_visited(bind.x2, bind.y2))
            continue;
        remove_wall(table, bind);
        add_visit(bind.x2, bind.y2);
        if (bind.x2 > 0)
        {
            if (!in_visited(bind.x2 - 1, bind.y2))
            {
                if (!add_bind(&frontier, bind.x2, bind.y2, bind.x2 - 1, bind.y2))
                    goto cleanup;
            }
        }
        if (bind.x2 < table.columns - 1)
        {
            if (!in_visited(bind.x2 + 1, bind.y2))
            {
                if (!add_bind(&frontier, bind.x2, bind.y2, bind.x2 + 1, bind.y2))
                    goto cleanup;
            }
        }
        if (bind.y2 > 0)
        {
            if (!in_visited(bind.x2, bind.y2 - 1))
            {
                if (!add_bind(&frontier, bind.x2, bind.y2, bind.x2, bind.y2 - 1))
                    goto cleanup;
            }
        }
        if (bind.y2 < table.rows - 1)
        {
            if (!in_visited(bind.x2, bind.y2 + 1))
            {
                if (!add_bind(&frontier, bind.x2, bind.y2, bind.x2, bind.y2 + 1))
                    goto cleanup;
            }
        }
    }

cleanup:

    destroy_Bind(&frontier);
    destroy_Visited();
    LABIRIT_STEPS_COMMIT();
    return;
}
