
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"

/*
    allocates a small block
    returns 1 on success
    returns 0 on failure
*/
int salloc(FILE *disk)
{
    fseek(disk, sizeof(int), SEEK_SET);
    
    int small_block_head;
    fwrite(&small_block_head, sizeof(int), 1, disk);
    
}

int add_user(FILE *disk, user u)
{

}


int main()
{
    FILE *disk = fopen("disk", "ab+");

    fclose(disk);
    return 0;
}