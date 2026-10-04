#include "../backend/all_algorithms.h"
#include "graph.h"

static bool same_text(const char *left, const char *right)
{
    if (!left || !right)
    {
        return false;
    }

    while (*left && *right)
    {
        if (*left != *right)
        {
            return false;
        }
        left++;
        right++;
    }

    return *left == '\0' && *right == '\0';
}

MazeAlgorithm parse_algorithm_name(const char *name)
{
    if (!name || same_text(name, "prim"))
    {
        return MAZE_ALGO_PRIM;
    }
    if (same_text(name, "dfs"))
    {
        return MAZE_ALGO_DFS;
    }
    if (same_text(name, "growing_tree"))
    {
        return MAZE_ALGO_GROWING_TREE;
    }
    if (same_text(name, "watson"))
    {
        return MAZE_ALGO_WATSON;
    }
    if (same_text(name, "binary"))
    {
        return MAZE_ALGO_BINARY;
    }
    if (same_text(name, "recursive") || same_text(name, "recursive_division"))
    {
        return MAZE_ALGO_RECURSIVE_DIVISION;
    }

    return MAZE_ALGO_PRIM;
}

const char *maze_algorithm_name(MazeAlgorithm algorithm)
{
    switch (algorithm)
    {
    case MAZE_ALGO_DFS:
        return "dfs";
    case MAZE_ALGO_GROWING_TREE:
        return "growing_tree";
    case MAZE_ALGO_WATSON:
        return "watson";
    case MAZE_ALGO_BINARY:
        return "binary";
    case MAZE_ALGO_RECURSIVE_DIVISION:
        return "recursive_division";
    case MAZE_ALGO_PRIM:
    default:
        return "prim";
    }
}

static void maze_add_entrance_exit(TABLE table)
{
    if (!table.data || table.rows <= 0 || table.columns <= 0)
    {
        return;
    }

    table.data[0][0].wall.top = 0;
    table.data[table.rows - 1][table.columns - 1].wall.bottom = 0;
}

void maze_run_algorithm(TABLE table, MazeAlgorithm algorithm)
{
    switch (algorithm)
    {
    case MAZE_ALGO_DFS:
        (void)dfs_algorithm(table);
        break;
    case MAZE_ALGO_GROWING_TREE:
        growing_tree_alg(table);
        break;
    case MAZE_ALGO_WATSON:
        watson_alg(table);
        break;
    case MAZE_ALGO_BINARY:
        binary_algos(table);
        break;
    case MAZE_ALGO_RECURSIVE_DIVISION:
        recursive_division_algorithm(table);
        break;
    case MAZE_ALGO_PRIM:
    default:
        prim_alg(table);
        break;
    }

    maze_add_entrance_exit(table);
}

void print_maze_info(TABLE table, MazeAlgorithm algorithm)
{
    printf("algorithm: %s\n", maze_algorithm_name(algorithm));
    printf("size: %d x %d\n", table.columns, table.rows);
    printf("seed: %d\n\n", table.seed);
}

void print_maze(TABLE table)
{
    if (!table.data || table.rows <= 0 || table.columns <= 0)
    {
        printf("maze is empty\n");
        return;
    }

    printf("+");
    for (int x = 0; x < table.columns; x++)
    {
        if (table.data[0][x].wall.top)
        {
            printf("---+");
        }
        else
        {
            printf("   +");
        }
    }
    printf("\n");

    for (int y = 0; y < table.rows; y++)
    {
        printf("|");
        for (int x = 0; x < table.columns; x++)
        {
            printf("   ");
            if (table.data[y][x].wall.right)
            {
                printf("|");
            }
            else
            {
                printf(" ");
            }
        }
        printf("\n");

        printf("+");
        for (int x = 0; x < table.columns; x++)
        {
            if (table.data[y][x].wall.bottom)
            {
                printf("---+");
            }
            else
            {
                printf("   +");
            }
        }
        printf("\n");
    }
}


int run_maze(int columns, int rows, unsigned int seed, MazeAlgorithm algorithm)
{
    TABLE table = init_table((unsigned int)columns, (unsigned int)rows, seed);
    if (!table.data)
    {
        fprintf(stderr, "Failed to allocate maze table\n");
        return 1;
    }

    maze_run_algorithm(table, algorithm);
    print_maze_info(table, algorithm);
    print_maze(table);
    clear_table(&table);
    return 0;
}
