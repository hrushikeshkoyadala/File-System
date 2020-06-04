//
// Created by Hrushikesh Koyadala on 11/05/20.
//

#define DISK_SIZE 52613349376
#define BIG_BLOCK_SIZE 80
#define SMALL_BLOCK_SIZE 40

typedef struct user
{
    int ID;
    char name[20];
    int message_start_offset;
    int next_user;
} user;

typedef struct small_message
{
    //1 - big ; 0 - smalls
    char block_size;

    char message_str[20];
    char timestamp[26];
    int sender_ID;
} small_message;