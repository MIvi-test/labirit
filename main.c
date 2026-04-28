#include <stdlib.h>
#include <time.h>
#include "backend/all_alg.h"
#include "frontend/graph.h"

int main(int argc, char **argv)
{
    int columns = 20;
    int rows = 10;
    unsigned int seed = (unsigned int)time(NULL);
    MazeAlgorithm algorithm = MAZE_ALGO_PRIM;
    bool ascii_mode = false;

    if (argc > 1)
    {
        if (strcmp(argv[1], "--ascii") == 0)
        {
            ascii_mode = true;
        }
        else
        {
            algorithm = parse_algorithm_name(argv[1]);
        }
    }
    if (argc > 2)
    {
        if (ascii_mode)
        {
            algorithm = parse_algorithm_name(argv[2]);
        }
        else
        {
            columns = atoi(argv[2]);
        }
    }
    if (argc > 3)
    {
        if (ascii_mode)
        {
            columns = atoi(argv[3]);
        }
        else
        {
            rows = atoi(argv[3]);
        }
    }
    if (argc > 4)
    {
        if (ascii_mode)
        {
            rows = atoi(argv[4]);
        }
        else
        {
            seed = (unsigned int)atoi(argv[4]);
        }
    }
    if (argc > 5 && ascii_mode)
    {
        seed = (unsigned int)atoi(argv[5]);
    }

    if (columns <= 0 || rows <= 0)
    {
        fprintf(stderr, "Usage: main.exe [algorithm] [columns] [rows] [seed]\n");
        fprintf(stderr, "   or: main.exe --ascii [algorithm] [columns] [rows] [seed]\n");
        return 1;
    }

    if (ascii_mode)
    {
        TABLE table = INIT_TABLE((unsigned int)columns, (unsigned int)rows, seed);
        if (!table.data)
        {
            fprintf(stderr, "Failed to allocate maze table\n");
            return 1;
        }

        maze_run_algorithm(table, algorithm);
        print_maze_info(table, algorithm);
        print_maze(table);
        CLEAR_TABLE(&table);
        return 0;
    }

    return run_maze_window(columns, rows, seed, algorithm);
}

