#include <stdlib.h>
#include <time.h>
#include "backend/all_alg.h"

int main()
{
    TABLE table;
    table = INIT_TABLE(10,20,201010);
    prim_alg(table);
    return 1;
};