#include <stdlib.h>
#include <time.h>
#include "backend/binart_tree_alg.c"
#include "backend/init.c"


int main()
{

    TABLE table;
    table = INIT_TABLE(5,10,10203040);


    binary_algos(table);
    // в table должен быть реализован лабиринт

    return 1;
}