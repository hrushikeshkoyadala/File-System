
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

user get_user_by_ID(FILE *disk, int required_ID)
{
    int user_head;
    fseek(disk, sizeof(int) * 4, SEEK_SET);
    fread(&user_head, sizeof(int), 1, disk);

    fseek(disk, user_head, SEEK_SET);

    user current_user;
    fread(&current_user, sizeof(user), 1, disk);

    while (current_user.ID != required_ID)
    {
        fseek(disk, current_user.next_user, SEEK_SET);
        fread(&current_user, sizeof(user), 1, disk);
    }

    return current_user;
}

void update_user(FILE *disk, user *to_update)
{
    int current_address;
    fseek(disk, sizeof(int) * 4, SEEK_SET);
    fread(&current_address, sizeof(int), 1, disk);

    user current_user;
    
    fseek(disk, current_address, SEEK_SET);
    fread(&current_user, sizeof(user), 1, disk);

    while (current_user.ID != to_update->ID)
    {
        current_address = current_user.next_user;
        fseek(disk, current_address, SEEK_SET);
        fread(&current_user, sizeof(user), 1, disk);
    }

    fseek(disk, current_address, SEEK_SET);
    fwrite(to_update, sizeof(user), 1, disk);
}


/*
    returns 1 on success
    returns 0 on failure
*/
int add_message(FILE *disk, user *receiver, message *msg)
{
    int message_address = balloc(disk);

    if (message_address == 0)
        return 0;

    msg->next_message = 0;
    
    //receiver has no messages
    if (receiver->message_start_offset == 0)
    { 
        receiver->message_start_offset = message_address;
        update_user(disk, receiver);
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
            update_user(disk, &current_user);
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

void display_messages(FILE *disk, user to_display)
{
    if (to_display.message_start_offset == 0)
        return;

    message current_message;
    fseek(disk, to_display.message_start_offset, SEEK_SET);
    fread(&current_message, sizeof(message), 1, disk);

    while (current_message.next_message != 0)
    {
        printf("%s\n", current_message.message_str);
        fseek(disk, current_message.next_message, SEEK_SET);
        fread(&current_message, sizeof(message), 1, disk);
    }

    printf("%s\n", current_message.message_str);
}

message create_message(char *message_str, char *sender_name)
{
    message to_return;
    strcpy(to_return.message_str, message_str);
    strcpy(to_return.sender_name, sender_name);
    to_return.next_message = 0;

    return to_return;
}

/*
    returns 1 on success
    returns 0 on failure
*/
int add_user_from_file(FILE *disk, char *user_name, FILE *messages)
{
    int user_ID = add_user(disk, user_name);

    if (user_ID == -1)
        return 0;
    
    user user_struct = get_user_by_ID(disk, user_ID);

    char str[55];
    message curr_message;
    while (fscanf(messages, "%[^\n]\n", str) >= 1)
    {
        curr_message = create_message(str, "Anonymous");

        if (add_message(disk, &user_struct, &curr_message) == 0)
            break;
    }

    return 1;
}

int main()
{
    FILE *disk = fopen("disk", "rb+");
    char user_names[][20] = {"Batman", "Superman", "Wonder Woman", "Flash", "Cyborg", "Green Lantern", "Martian manhunter", "Constantine"};
    char message_files[][100] = {"/Users/hrushi/CLionProjects/File System/test_data/batman_messages.txt", "/Users/hrushi/CLionProjects/File System/test_data/superman_messages.txt", "/Users/hrushi/CLionProjects/File System/test_data/ww_messages.txt", "/Users/hrushi/CLionProjects/File System/test_data/flash_messages.txt", "/Users/hrushi/CLionProjects/File System/test_data/cyborg_messages.txt", "/Users/hrushi/CLionProjects/File System/test_data/green_messages.txt", "/Users/hrushi/CLionProjects/File System/test_data/martian_messages.txt", "/Users/hrushi/CLionProjects/File System/test_data/constantine_messages.txt"};
    FILE *message_file;

    for (int i = 0; i < 3; i++)
    {
        message_file = fopen(message_files[i], "r");
        if (message_file != NULL) {
            if (add_user_from_file(disk, user_names[i], message_file) == 0)
                break;
        } else
        {
            printf("Aegg");
            break;
        }
        fclose(message_file);
    }
    display_users(disk);
    for (int i = 0; i < 3; i++)
    {
        display_messages(disk, get_user_by_ID(disk, i));
        printf("\n\n**************************************************************\n\n");
    }
    fclose(disk);
    return 0;
}