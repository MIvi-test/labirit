#include "../deter.h"

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

typedef struct VISITED
{
    int x;
    int y;
    struct VISITED *before;
} Visited;

Visited *GLOBAL_VISITED_PTR = NULL;

static void destroy_Bind(void)
{
    while (GLOBAL_BIND_PTR)
    {
        Bind *temp = GLOBAL_BIND_PTR->before;
        free(GLOBAL_BIND_PTR);
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
    massive r;
    {
        r.x1 = 0;
        r.y1 = 0;
        r.x2 = 0;
        r.y2 = 0;
    }
    if (!GLOBAL_BIND_PTR)
    {
        return r;
    }

    Bind *prev = NULL;
    Bind *node = GLOBAL_BIND_PTR;

    while (node->before != NULL && (rand() % 4) != 0)
    {
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
    return r;
}

bool add_visit(int x, int y)
{
    Visited *new_element = (Visited *)malloc(sizeof(Visited));
    if (!new_element)
    {
        return false;
    }
    new_element->x = x;
    new_element->y = y;
    new_element->before = GLOBAL_VISITED_PTR;
    GLOBAL_VISITED_PTR = new_element;
    return true;
}

void destroy_Visited()
{
    while (GLOBAL_VISITED_PTR)
    {
        Visited *temp = GLOBAL_VISITED_PTR->before;
        free(GLOBAL_VISITED_PTR);
        GLOBAL_VISITED_PTR = temp;
    }
}

bool in_visited(int x, int y)
{
    Visited *cur = GLOBAL_VISITED_PTR;
    while (cur)
    {
        if (cur->x == x && cur->y == y)
        {
            return true;
        }
        cur = cur->before;
    }
    return false;
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
    destroy_Bind();
    destroy_Visited();

    add_visit(0, 0);
    add_bind(0, 0, 0, 1);
    add_bind(0, 0, 1, 0);
    do
    {
        massive bind = pop_bind();
        if (!in_visited(bind.x2, bind.y2))
        {
            remove_wall(table, bind);
            add_visit(bind.x2, bind.y2);
        }
        if (bind.x2 > 0)
        {
            if (!in_visited(bind.x2 - 1, bind.y2))
            {
                add_bind(bind.x2, bind.y2, bind.x2 - 1, bind.y2);
            }
        }
        if (bind.x2 < table.columns - 1)
        {
            if (!in_visited(bind.x2 + 1, bind.y2))
            {
                add_bind(bind.x2, bind.y2, bind.x2 + 1, bind.y2);
            }
        }
        if (bind.y2 > 0)
        {
            if (!in_visited(bind.x2, bind.y2 - 1))
            {
                add_bind(bind.x2, bind.y2, bind.x2, bind.y2 - 1);
            }
        }
        if (bind.y2 < table.rows - 1)
        {
            if (!in_visited(bind.x2, bind.y2 + 1))
            {
                add_bind(bind.x2, bind.y2, bind.x2, bind.y2 + 1);
            }
        }
    } while (GLOBAL_BIND_PTR);

    destroy_Bind();
    destroy_Visited();
    return;
}