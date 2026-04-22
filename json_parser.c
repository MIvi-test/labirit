#include "json_parser.h"
#include "analisys/benchmarks_runner.h"

#include <stdio.h>

static bool dump_json_file(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        fprintf(stderr, "cannot open %s\n", path);
        return false;
    }

    printf("\n== %s ==\n", path);
    int ch;
    while ((ch = fgetc(f)) != EOF)
    {
        putchar(ch);
    }
    if (ferror(f))
    {
        fclose(f);
        return false;
    }
    if (ferror(stdout))
    {
        fclose(f);
        return false;
    }
    putchar('\n');
    fclose(f);
    return true;
}

bool dump_all_json_to_console(void)
{
    bool ok = true;
    for (int i = 0; i < CREATED_JSON_FILES_COUNT; i++)
    {
        const char *path = CREATED_JSON_FILES[i];
        ok = dump_json_file(path) && ok;
    }
    return ok;
}
