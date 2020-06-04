
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
    int memory_probe;

    fseek(disk, sizeof(int) * 2, SEEK_SET);
    fread(&memory_probe, sizeof(int), 1, disk);

    //no fragments, allocating from free memory pool
    if (memory_probe == 0)
    {
        int free_memory_pool;
        fseek(disk, sizeof(int), SEEK_SET);
        fread(&free_memory_pool, sizeof(int), 1, disk);

        //not enough space
        if (free_memory_pool + SMALL_BLOCK_SIZE >= DISK_SIZE)
            return 0;

        memory_probe = free_memory_pool;
        
        free_memory_pool += SMALL_BLOCK_SIZE;
        fseek(disk, sizeof(int), SEEK_SET);
        fwrite(&free_memory_pool, sizeof(int), 1, disk);
    }
    //allocating a fragment
    else
    {
        int next_free_block;
        fseek(disk, memory_probe, SEEK_SET);
        fread(&next_free_block, sizeof(int), 1, disk);

        //updating small block free list header
        fseek(disk, sizeof(int) * 2, disk);
        fwrite(&next_free_block, sizeof(int), 1, disk);
    }
    
    return memory_probe;
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