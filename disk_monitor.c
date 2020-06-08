//
// Created by Hrushikesh Koyadala on 08/06/20.
//

#include <stdio.h>
#include <string.h>
#include "user.h"


int main()
{
    FILE *disk = fopen("disk", "rb");

    int read_variable;

    fread(&read_variable, sizeof(int), 1, disk);
    printf("next user ID = %d\n", read_variable);

    fread(&read_variable, sizeof(int), 1, disk);
    printf("free memory pool = %d\n", read_variable);

    fread(&read_variable, sizeof(int), 1, disk);
    printf("small chunk = %d\n", read_variable);

    fread(&read_variable, sizeof(int), 1, disk);
    printf("big chunk = %d\n", read_variable);

    fread(&read_variable, sizeof(int), 1, disk);
    printf("user start = %d\n", read_variable);

    fclose(disk);
    return 0;
}