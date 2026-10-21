#include "../deter.h"
#include <limits.h>

typedef struct massive
{
    int x1;
    int y1;
    int x2;
    int y2;
} massive;

typedef struct BIND
{
    massive bind;
    struct BIND *before;
} Bind;

Bind *GLOBAL_BIND_PTR = NULL;

static unsigned char *visited;
static int visited_columns;
static size_t visited_bytes;

static void destroy_Bind(void)
{
    while (GLOBAL_BIND_PTR)
    {
        Bind *temp = GLOBAL_BIND_PTR->before;
        free(GLOBAL_BIND_PTR);
        labirit_metrics_free(sizeof(Bind));
        GLOBAL_BIND_PTR = temp;
    }
}

bool add_bind(int x1, int y1, int x2, int y2)
{
    Bind *new_element = (Bind *)malloc(sizeof(Bind));
    if (!new_element)
    {
        return false;
    }
    labirit_metrics_alloc(sizeof(Bind));
    new_element->before = GLOBAL_BIND_PTR;
    GLOBAL_BIND_PTR = new_element;
    new_element->bind.x1 = x1;
    new_element->bind.y1 = y1;
    new_element->bind.x2 = x2;
    new_element->bind.y2 = y2;
    return true;
}

massive pop_bind()
{
    LABIRIT_STEPS_DECLARE();
    massive r;
    {
        r.x1 = 0;
        r.y1 = 0;
        r.x2 = 0;
        r.y2 = 0;
    }
    if (!GLOBAL_BIND_PTR)
    {
        LABIRIT_STEPS_COMMIT();
        return r;
    }

    Bind *prev = NULL;
    Bind *node = GLOBAL_BIND_PTR;

    while (node->before != NULL && (rand() % 4) != 0)
    {
        LABIRIT_STEPS_ADD(1);
        prev = node;
        node = node->before;
    }

    r = node->bind;

    if (prev == NULL)
    {
        GLOBAL_BIND_PTR = node->before;
    }
    else
    {
        prev->before = node->before;
    }

    free(node);
    labirit_metrics_free(sizeof(Bind));
    LABIRIT_STEPS_COMMIT();
    return r;
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
    destroy_Bind();
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
    if (table.rows > 1 && !add_bind(0, 0, 0, 1))
        goto cleanup;
    if (table.columns > 1 && !add_bind(0, 0, 1, 0))
        goto cleanup;
    while (GLOBAL_BIND_PTR)
    {
        LABIRIT_STEPS_ADD(1);
        massive bind = pop_bind();
        if (in_visited(bind.x2, bind.y2))
            continue;
        remove_wall(table, bind);
        add_visit(bind.x2, bind.y2);
        if (bind.x2 > 0)
        {
            if (!in_visited(bind.x2 - 1, bind.y2))
            {
                if (!add_bind(bind.x2, bind.y2, bind.x2 - 1, bind.y2))
                    goto cleanup;
            }
        }
        if (bind.x2 < table.columns - 1)
        {
            if (!in_visited(bind.x2 + 1, bind.y2))
            {
                if (!add_bind(bind.x2, bind.y2, bind.x2 + 1, bind.y2))
                    goto cleanup;
            }
        }
        if (bind.y2 > 0)
        {
            if (!in_visited(bind.x2, bind.y2 - 1))
            {
                if (!add_bind(bind.x2, bind.y2, bind.x2, bind.y2 - 1))
                    goto cleanup;
            }
        }
        if (bind.y2 < table.rows - 1)
        {
            if (!in_visited(bind.x2, bind.y2 + 1))
            {
                if (!add_bind(bind.x2, bind.y2, bind.x2, bind.y2 + 1))
                    goto cleanup;
            }
        }
    }

cleanup:

    destroy_Bind();
    destroy_Visited();
    LABIRIT_STEPS_COMMIT();
    return;
}
