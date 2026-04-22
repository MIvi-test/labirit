#include <stdio.h>
#include <string.h>
#include "analisys/benchmarks_runner.h"
#include "json_parser.h"

/*
 * В проекте генераторы подключаются через all_algorithms.h (include .c),
 * поэтому модули анализа подключаются как единицы трансляции здесь.
 */
#include "analisys/time_wait_benchmarks.c"
#include "analisys/topology_benchmarks.c"
#include "json_parser.c"

int main(int argc, char **argv)
{
    if (argc >= 2 && strcmp(argv[1], "--dump") == 0)
    {
        return dump_all_json_to_console() ? 0 : 1;
    }

    if (!ensure_results_dir())
    {
        fprintf(stderr, "cannot create analisys/json_data\n");
        return 1;
    }

    if (!run_time_benchmarks())
    {
        fprintf(stderr, "time benchmarks failed\n");
        return 1;
    }

    if (!run_topology_benchmarks())
    {
        fprintf(stderr, "topology benchmarks failed\n");
        return 1;
    }

    printf("Created json files:\n");
    for (int i = 0; i < CREATED_JSON_FILES_COUNT; i++)
    {
        printf("%s\n", CREATED_JSON_FILES[i]);
    }

    return 0;
}

