#ifndef DETER_H
#define DETER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

typedef union
{
    unsigned char all_bits;
    struct
    {
        unsigned char top : 1;
        unsigned char right : 1;
        unsigned char bottom : 1;
        unsigned char left : 1;
        unsigned char unused : 4;
    } wall;
} Cell;

typedef struct
{
    Cell **data;
    int rows;
    int columns;
    int seed;

} TABLE;

typedef TABLE MazeTable;

#endif // DETER_H