#include "benchmarks_runner.h"
#include <stdio.h>

int main(void)
{
    if (!ensure_results_dir())
    {
        fprintf(stderr, "labirit: cannot create analisys/json_data\n");
        return 1;
    }
    if (!run_combined_benchmarks())
    {
        fprintf(stderr, "labirit: combined benchmarks failed\n");
        return 1;
    }
    return 0;
}
