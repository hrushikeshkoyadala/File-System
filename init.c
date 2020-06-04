//
// Created by Hrushikesh Koyadala on 11/05/20.
//

#include <stdio.h>
#include "user.h"

int main()
{
    FILE *disk = fopen("disk", "wb");
    int write_variable;

    //number of users = 0
    write_variable = 0;
    fwrite(&write_variable, sizeof(int), 1, disk);

    //small block free list head pointer
    write_variable = sizeof(int)*3;
    fwrite(&write_variable, sizeof(int), 1, disk);

    //big block free list head pointer
    write_variable = sizeof(int)*3 + SMALL_BLOCK_SIZE;
    fwrite(&write_variable, sizeof(int), 1, disk);

    fclose(disk);
    return 0;
}