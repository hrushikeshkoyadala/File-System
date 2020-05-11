//
// Created by Hrushikesh Koyadala on 11/05/20.
//

#include <stdio.h>

int main()
{
    int no_of_users = 0;
    FILE *disk = fopen("disk", "wb");
    fseek(disk, 0, SEEK_SET);
    fwrite(&no_of_users, sizeof(int), 1, disk);
    fclose(disk);
}