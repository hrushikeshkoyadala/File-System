//
// Created by Hrushikesh Koyadala on 11/05/20.
//

#include <stdio.h>
#include "user.h"

int main()
{
    FILE *disk = fopen("disk", "wb");
    int write_variable;

    //next user ID
    write_variable = 0;
    fwrite(&write_variable, sizeof(int), 1, disk);

    //free memory pool starting address
    write_variable = sizeof(int)*5;
    fwrite(&write_variable, sizeof(int), 1, disk);

    //small block free list head pointer
    write_variable = 0;
    fwrite(&write_variable, sizeof(int), 1, disk);

    //big block free list head pointer
    write_variable = 0;
    fwrite(&write_variable, sizeof(int), 1, disk);

    //head address of user list
    write_variable = 0;
    fwrite(&write_variable, sizeof(int), 1, disk);

    fclose(disk);
    return 0;
}