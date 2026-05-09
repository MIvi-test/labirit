#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "backend/all_algorithms.h"
#include "frontend/graph.h"
#include "json_parser.h"

#define DEFAULT_VALUE_COLUMNS 10
#define DEFAULT_VALUE_ROWS 10

static void print_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s [options] [algorithm] [columns] [rows] [seed]\n"
            "  algorithm: prim | dfs | growing_tree | watson | binary | recursive\n"
            "  options:\n"
            "    --ascii, -a     text output (console); on Windows skips the GUI\n"
            "    --dump          print generated JSON files to stdout\n"
            "    --help, -h      this message\n",
            prog);
}

static int parse_positive_int(const char *s, int fallback)
{
    if (!s)
    {
        return fallback;
    }
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (end == s || v < 1)
    {
        return fallback;
    }
    if (v > 10000)
    {
        return fallback;
    }
    return (int)v;
}

int main(int argc, char **argv)
{
    int columns = DEFAULT_VALUE_COLUMNS;
    int rows = DEFAULT_VALUE_ROWS;
    unsigned int seed = (unsigned int)time(NULL);
    MazeAlgorithm algorithm = MAZE_ALGO_PRIM;
    bool ascii_mode = false;

    int i = 1;
    while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0')
    {
        if (strcmp(argv[i], "--dump") == 0)
        {
            if (!dump_all_json_to_console())
            {
                return 1;
            }
            return 0;
        }
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            print_usage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "--ascii") == 0 || strcmp(argv[i], "-a") == 0)
        {
            ascii_mode = true;
            i++;
            continue;
        }
        fprintf(stderr, "%s: unknown option %s\n", argv[0], argv[i]);
        print_usage(argv[0]);
        return 2;
    }

    if (i < argc)
    {
        algorithm = parse_algorithm_name(argv[i]);
        i++;
    }
    if (i < argc)
    {
        columns = parse_positive_int(argv[i], columns);
        i++;
    }
    if (i < argc)
    {
        rows = parse_positive_int(argv[i], rows);
        i++;
    }
    if (i < argc)
    {
        seed = (unsigned int)parse_positive_int(argv[i], (int)seed);
        i++;
    }
    if (i < argc)
    {
        fprintf(stderr, "%s: extra arguments after seed\n", argv[0]);
        print_usage(argv[0]);
        return 2;
    }

#ifdef _WIN32
    if (ascii_mode)
    {
        MazeTable table = init_table((unsigned int)columns, (unsigned int)rows, seed);
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
#else
    (void)ascii_mode;
#endif

    return run_maze_window(columns, rows, seed, algorithm);
}
