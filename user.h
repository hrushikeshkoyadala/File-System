//
// Created by Hrushikesh Koyadala on 11/05/20.
//

#define DISK_SIZE 10485760
#define BIG_BLOCK_SIZE 100
#define SMALL_BLOCK_SIZE 35

typedef struct user
{
    int ID;
    char name[20];
    int message_start_offset;
    int next_user;
} user;

typedef struct message
{
    char message_str[50];
    char timestamp[17];
    char sender_name[20];
} message;