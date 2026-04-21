#include "../deter.h"

typedef struct stack
{
    int x;
    int y;
    struct stack *before;
} stack;

bool push(stack **path, int x, int y)
{
    stack *new_element = malloc(sizeof(stack));
    if (!new_element)
    {
        return false;
    }

    new_element->x = x;
    new_element->y = y;
    new_element->before = *path;
    *path = new_element;

    return true;
}

bool pop(stack **path, int *x, int *y)
{
    if (!path || !*path)
    {
        return false;
    }

    stack *temp = *path;

    if (x)
        *x = temp->x;
    if (y)
        *y = temp->y;

    *path = temp->before;
    free(temp);

    return true;
}

void dfs_reallisation(TABLE table, stack **path, int x, int y, bool **visited)
{

    int r = rand();
    /*
        0
       3 1
        2
    */
   int new_x = x;
   int new_y = y;
    switch (r % 4)
    {
    case 3:
        if (x - 1 >= 0 && visited[y][x - 1] == false)
        {
            table.data[y][x].wall.left = 0;
            table.data[y][x - 1].wall.right = 0;
            new_x = x-1;
            break;
        }
    case 2:
        if (y + 1 < table.rows && visited[y + 1][x] == false)
        {
            table.data[y][x].wall.bottom = 0;
            table.data[y + 1][x].wall.top = 0;
            new_y = y+1;
            break;
        }
    case 1:
        if (x + 1 < table.columns && visited[y][x + 1] == false)
        {
            table.data[y][x].wall.right = 0;
            table.data[y][x + 1].wall.left = 0;
            new_x = x+1;
            break;
        }
    case 0:
        if (y - 1 >= 0 && visited[y - 1][x] == false)
        {
            table.data[y][x].wall.top = 0;
            table.data[y - 1][x].wall.bottom = 0;
            new_y = y-1;
            break;

        }
    default:
        if (!pop(&path, &new_x, &new_y))
        {
            // значит всё готово
            return;
        }
    }
    visited[y][x] = true;
    if (!push(&path, x, y))
    {
        printf("ну тут память надо почистить мб, хз, подумаю ещё");
        return;
    }
    dfs_reallisation(table,path,new_x,new_y,visited);
}

bool DFS_alg(TABLE table)
{
    /*
 Выбрать любую начальную клетку, отметить ее как посещенную, отправить в стек
 Пока стек не пуст:
 Достать из стека клетку и считать ее текущей
 Если среди четырех клеток, смежных с текущей, имеются те, которые еще не были посещены:
 Отправить текущую клетку в стек
 Случайным образом выбрать одну из соседних непосещенных клеток
 Удалить стенку между текущей и выбранной клеткой
 Отметить выбранную клетку как посещенную и отправить ее в стек
 */
    stack *path = NULL;
    int start_x = 0;
    int start_y = 0;

    bool **visited = (bool **)malloc(sizeof(bool *) * table.columns);
    if (!visited)
    {
        return false;
    }
    for (int j = 0; j < table.columns; j++)
    {
        bool *row = (bool *)calloc(table.rows, sizeof(bool));
        if (!row)
        {
            for (int n = j - 1; n >= 0; n--)
            {
                free(visited[n]);
            }
            free(visited);
            return false;
        }
        visited[j] = row;
    }

    dfs_reallisation(table, &path, start_x, start_y, visited);
    while(pop(&path,NULL,NULL))
    {
        ;
    }
    for(int n = 0; n < table.columns; n++)
    {
        free(visited[n]);
    }
    free(visited);
    
    return true;
}


// тут можно заменить хеш таблицу на на линейный список, и тогда с О(1) будет О(n) по времени
// пространственная сложность сохранится.