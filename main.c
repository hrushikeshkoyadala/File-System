
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
        fseek(disk, sizeof(int) * 2, SEEK_SET);
        fwrite(&next_free_block, sizeof(int), 1, disk);
    }
    
    return memory_probe;
}

/*
    returns user ID on success
    returns -1 on failure
*/
int add_user(FILE *disk, char *name_str)
{
    int allocated_memory = salloc(disk);
    
    //not enough space on disk
    if (allocated_memory == 0)
            return -1;

    int next_user_ID;
    fseek(disk, 0, SEEK_SET);
    fread(&next_user_ID, sizeof(int), 1, disk);

    user new_user;
    strcpy(new_user.name, name_str);
    new_user.message_start_offset = 0;
    new_user.ID = next_user_ID;

    int first_user_node;
    fseek(disk, sizeof(int)*4, SEEK_SET);
    fread(&first_user_node, sizeof(int), 1, disk);

    if (first_user_node == 0)
    {
        new_user.next_user = 0;
        
        //offset for first user
        fseek(disk, sizeof(int) * 4, SEEK_SET);
        fwrite(&allocated_memory, sizeof(int), 1, disk);
    }
    else
    {
        //updating new user as head node of user list
        fseek(disk, sizeof(int) * 4, SEEK_SET);
        fread(&new_user.next_user, sizeof(int), 1, disk);

        fseek(disk, sizeof(int) * 4, SEEK_SET);
        fwrite(&allocated_memory, sizeof(int), 1, disk);
    }

    //writing user to disk
    fseek(disk, allocated_memory, SEEK_SET);
    fwrite(&new_user, sizeof(user), 1, disk);

    //update number of users
    next_user_ID++;
    fseek(disk, 0, SEEK_SET);
    fwrite(&next_user_ID, sizeof(int), 1, disk);

    return new_user.ID;
}

void free_sblock(FILE *disk, int block_address)
{
    int block_list_head;
    fseek(disk, sizeof(int) * 2, SEEK_SET);
    fread(&block_list_head, sizeof(int), 1, disk);

    fseek(disk, block_address, SEEK_SET);
    fwrite(&block_list_head, sizeof(int), 1, disk);

    fseek(disk, sizeof(int) * 2, SEEK_SET);
    fwrite(&block_address, sizeof(int), 1, disk);
}

void update_user(FILE *disk, user *u, int user_address)
{
    fseek(disk, user_address, SEEK_SET);
    fwrite(u, sizeof(user), 1, disk);
}
/*
    returns 1 on success
    returns 0 on failure
*/
int delete_user_by_ID(FILE *disk, int remove_ID)
{
    int user_head;
    fseek(disk, sizeof(int) * 4, SEEK_SET);
    fread(&user_head, sizeof(int), 1, disk);

    //no users on disk
    if (user_head == 0)
        return 0;

    user current_user;
    fseek(disk, user_head, SEEK_SET);
    fread(&current_user, sizeof(user), 1, disk);

    if (current_user.ID == remove_ID)
    {
        //clear_messages;
        fseek(disk, sizeof(int) * 4, SEEK_SET);
        fwrite(&current_user.next_user, sizeof(int), 1, disk);
        free_sblock(disk, user_head);
        return 1;
    }

    user next;
    int current_user_address = user_head;

    while (current_user.next_user != 0)
    {
        fseek(disk, current_user.next_user, SEEK_SET);
        fread(&next, sizeof(user), 1, disk);

        if (next.ID == remove_ID)
        {
            //clear_messages();
            int to_remove_block = current_user.next_user;
            current_user.next_user = next.next_user;
            update_user(disk, &current_user, current_user_address);
            free_sblock(disk, to_remove_block);
            return 1;
        }
        else
        {
            current_user = next;
            current_user_address = current_user.next_user;
        }
    }

    return 0;
}

/*
    assumes there is atleast one user
    change later
*/
void display_users(FILE *disk)
{
    int head_address;
    fseek(disk, sizeof(int) * 4, SEEK_SET);
    fread(&head_address, sizeof(int), 1, disk);
    printf("%d\n", head_address);
    if (head_address == 0)
        return;

    fseek(disk, head_address, SEEK_SET);    
    user u;
    fread(&u, sizeof(user), 1, disk);

    while (u.next_user != 0)
    {
        printf("%s %d\n", u.name, u.ID);
        fseek(disk, u.next_user, SEEK_SET);
        fread(&u, sizeof(user), 1, disk);
    }

    printf("%s %d\n", u.name, u.ID);
}

int main()
{
    FILE *disk = fopen("disk", "rb+");
    fclose(disk);
    return 0;
}