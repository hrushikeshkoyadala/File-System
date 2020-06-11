
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
    allocates a big block
    returns 1 on success
    returns 0 on failure
*/
int balloc(FILE *disk)
{
    int memory_probe;

    fseek(disk, sizeof(int) * 3, SEEK_SET);
    fread(&memory_probe, sizeof(int), 1, disk);

    //no fragments, allocating from free memory pool
    if (memory_probe == 0)
    {
        int free_memory_pool;
        fseek(disk, sizeof(int), SEEK_SET);
        fread(&free_memory_pool, sizeof(int), 1, disk);

        //not enough space
        if (free_memory_pool + BIG_BLOCK_SIZE >= DISK_SIZE)
            return 0;

        memory_probe = free_memory_pool;
        
        free_memory_pool += BIG_BLOCK_SIZE;
        fseek(disk, sizeof(int), SEEK_SET);
        fwrite(&free_memory_pool, sizeof(int), 1, disk);
    }
    //allocating a fragment
    else
    {
        int next_free_block;
        fseek(disk, memory_probe, SEEK_SET);
        fread(&next_free_block, sizeof(int), 1, disk);
        
        //updating big block free list header
        fseek(disk, sizeof(int) * 3, SEEK_SET);
        fwrite(&next_free_block, sizeof(int), 1, disk);
    }
    
    return memory_probe;
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

/*
    returns user address by ID
    returns -1 if user does not exist
*/
int get_address_by_ID(FILE *disk, int required_ID)
{
    int current_address;
    fseek(disk, sizeof(int) * 4, SEEK_SET);
    fread(&current_address, sizeof(int), 1, disk);

    if (current_address == 0)
        return -1;
    
    user current_user;
    fseek(disk, current_address, SEEK_SET);
    fread(&current_user, sizeof(user), 1, disk);

    while (current_user.ID != required_ID)
    {
        current_address = current_user.next_user;

        if (current_address == 0)
            return -1;
        
        fseek(disk, current_address, SEEK_SET);
        fread(&current_user, sizeof(user), 1, disk);
    }

    return current_address;
}

user* get_user_by_ID(FILE *disk, int required_ID)
{
    int user_address = get_address_by_ID(disk, required_ID);

    if (user_address == -1)
        return NULL;
    
    user *to_return = (user *)malloc(sizeof(user));
    fseek(disk, user_address, SEEK_SET);
    fread(to_return, sizeof(user), 1, disk);

    return to_return;
}

/*
    returns 1 on success
    returns 0 on failure
*/
int update_user(FILE *disk, user *to_update, int user_address)
{
    if (user_address == 0)
        return 0;

    fseek(disk, user_address, SEEK_SET);
    fwrite(to_update, sizeof(user), 1, disk);

    return 1;
}

/*
    returns 1 on success
    returns 0 on failure
*/
int add_message(FILE *disk, user *receiver, message *msg)
{
    int message_address = balloc(disk);

    if (receiver == NULL || msg == NULL || message_address == 0)
        return 0;

    msg->next_message = 0;
    
    //receiver has no messages
    if (receiver->message_start_offset == 0)
    { 
        receiver->message_start_offset = message_address;
    
        if (update_user(disk, receiver, get_address_by_ID(disk, receiver->ID)) == 0)
            return 0;
    }
    else
    {
        message current_message;
        int current_address = receiver->message_start_offset;
        fseek(disk, current_address, SEEK_SET);
        fread(&current_message, sizeof(message), 1, disk);

        while (current_message.next_message != 0)
        {
            current_address = current_message.next_message;
            fseek(disk, current_address, SEEK_SET);
            fread(&current_message, sizeof(message), 1, disk);
        }

        current_message.next_message = message_address;
        fseek(disk, current_address, SEEK_SET);
        fwrite(&current_message, sizeof(message), 1, disk);

        fseek(disk, message_address, SEEK_SET);
        fwrite(&message_address, sizeof(message), 1, disk);
    }

    fseek(disk, message_address, SEEK_SET);
    fwrite(msg, sizeof(message), 1, disk);

    return 1;
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
            
            if (update_user(disk, &current_user, get_address_by_ID(disk, current_user.ID)) == 0)
                return 0;

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


//Modify for more attributes
void display_users(FILE *disk)
{
    int head_address;
    fseek(disk, sizeof(int) * 4, SEEK_SET);
    fread(&head_address, sizeof(int), 1, disk);

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

void display_messages(FILE *disk, user *to_display)
{
    if (to_display == NULL || to_display->message_start_offset == 0)
        return;

    message current_message;
    fseek(disk, to_display->message_start_offset, SEEK_SET);
    fread(&current_message, sizeof(message), 1, disk);

    while (current_message.next_message != 0)
    {
        printf("%s\n", current_message.message_str);
        fseek(disk, current_message.next_message, SEEK_SET);
        fread(&current_message, sizeof(message), 1, disk);
    }

    printf("%s\n", current_message.message_str);
}

void display_messages_by_ID(FILE *disk, int display_ID)
{
    user *to_display = get_user_by_ID(disk, display_ID);

    if (to_display == NULL)
        return;

    display_messages(disk, to_display);
}

message* create_message(char *content, char *sender)
{
    message *new = (message *)malloc(sizeof(message));

    strcpy(new->message_str, content);
    strcpy(new->sender_name, sender);

    return new;
}

/*int main()
{
    FILE *disk = fopen("disk", "rb+");
    fclose(disk);
}*/