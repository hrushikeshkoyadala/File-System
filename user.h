//
// Created by Hrushikesh Koyadala on 11/05/20.
//

#define USER_LIMIT 5
#define MESSAGE_SIZE 50
#define MESSAGE_LIMIT 10

typedef struct user
{
    int ID;
    char name[20];
    int message_offset;
    int no_of_messages;
} user;

typedef struct message
{
    char message_str[50];
    char timestamp[26];
} message;