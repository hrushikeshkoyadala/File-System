#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "user.h"

int load_no_of_users(FILE *disk)
{
    fseek(disk, 0, SEEK_SET);
    int no_of_users;
    fread(&no_of_users, sizeof(int), 1, disk);

    return no_of_users;
}

user* load_users(FILE *disk, int no_of_users)
{
    user *users = (user *)malloc(sizeof(user) * USER_LIMIT);
    fseek(disk, sizeof(int), SEEK_SET);
    fread(users, sizeof(user), no_of_users, disk);

    return users;
}

void add_user(user *users, int no_of_users, char *name_str)
{
    user *new_user = users + no_of_users;
    new_user->ID = no_of_users;
    new_user->no_of_messages = 0;

    new_user->message_offset = sizeof(int) + USER_LIMIT*sizeof(user) + no_of_users*MESSAGE_LIMIT*MESSAGE_SIZE + 1;

    int i = 0;
    while (name_str[i])
    {
        new_user->name[i] = name_str[i];
        i++;
    }

    new_user->name[i] = '\0';
}

void add_message(FILE *disk, user *sender, char *message, user *receiver)
{
    message[0] = (char)(sender->ID + 48);
    fseek(disk, receiver->message_offset + (receiver->no_of_messages)*MESSAGE_SIZE, SEEK_SET);
    fwrite(message, MESSAGE_SIZE, 1, disk);
    receiver->no_of_messages++;
}

void print_messages(FILE *disk, user *users, user *user)
{
    fseek(disk, user->message_offset, SEEK_SET);
    char *message = (char *)malloc(50);

    for (int i = 0; i < user->no_of_messages; i++)
    {
        fread(message, MESSAGE_SIZE, 1, disk);
        printf("\"%s\" - %s\n", message, users[(int)message[0] - 48].name);
    }
}

void display_users(user *users, int no_of_users)
{
    if (no_of_users == 0)
        printf("no users\n");
    else
    {
        printf("ID\tName\n\n");
        for (int i = 0; i < no_of_users; i++)
            printf("%d\t%s\n", users[i].ID, users[i].name);
    }
}

void save_users(FILE *disk, user *users, int no_of_users)
{
    fseek(disk, 0, SEEK_SET);
    fwrite(&no_of_users, sizeof(int), 1, disk);
    fwrite(users, sizeof(user), no_of_users, disk);
}

int main()
{
    FILE *disk = fopen("disk", "rb+");
    int no_of_users = load_no_of_users(disk);
    user *users = load_users(disk, no_of_users);
    fclose(disk);
    return 0;
}
