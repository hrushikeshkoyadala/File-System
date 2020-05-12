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

user* get_user_by_ID(user *users, int ID)
{
    return users +ID;
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

void display_messages(FILE *disk, user *users, user *user)
{
    if (user->no_of_messages == 0)
    {
        printf("no messages\n");
        return;
    }

    fseek(disk, user->message_offset, SEEK_SET);
    char *message = (char *)malloc(50);

    for (int i = 0; i < user->no_of_messages; i++)
    {
        fread(message, MESSAGE_SIZE, 1, disk);
        printf("\"%s\" - %s\n", message + 1, users[(int)message[0] - 48].name);
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

    int choice = -1, ID = -1;

    while (choice != 8)
    {
        printf("\n\n*******************\n\n");
        printf("1.Login\n");
        printf("2.Register\n");
        printf("3.Your page\n");
        printf("4.Display a user's page\n");
        printf("5.Send message\n");
        printf("6.Comment on other user\n");
        printf("7.Logout\n");
        printf("8.Exit\n\n");

        scanf("%d", &choice);
        printf("\n");

        switch (choice)
        {
            case 1:
            {
                if (ID != -1 && ID < no_of_users)
                    printf("logging out\n");
                
                printf("Enter user ID : ");
                scanf("%d", &ID);

                if (no_of_users == 0 || ID >= no_of_users)
                {
                    printf("no users found\n");
                    ID = -1;
                }
                else
                    printf("Logged in\n");
                
                break;
            }

            case 2:
            {
                if (no_of_users == USER_LIMIT)
                    printf("user limit reached\n");
                else
                {
                    char name[20];
                    printf("Enter username : ");
                    scanf("\n%19[^\n]", name);

                    add_user(users, no_of_users, name);
                    printf("Your user ID is %d\n", no_of_users++);
                }

                break;
            }

            case 3:
            {
                if (ID == -1)
                    printf("not logged in\n");
                else
                    display_messages(disk, users, get_user_by_ID(users, ID));

                break;
            }

            case 4:
            {
                int to_display_ID;

                printf("Enter user ID to print : ");
                scanf("%d", &to_display_ID);

                if (to_display_ID < 0 || no_of_users == 0 || to_display_ID >= no_of_users)
                    printf("no users found\n");
                else
                    display_messages(disk, users, get_user_by_ID(users, to_display_ID));
                
                break;
            }

            case 5:
            {
                if (ID == -1)
                    printf("not logged in\n");
                else
                {
                    user *sender = get_user_by_ID(users, ID);

                    if (sender->no_of_messages >= MESSAGE_LIMIT)
                        printf("message limit reached\n");
                    else
                    {
                        char message[50];
                        printf("Enter message : ");
                        scanf("\n%48[^\n]", message + 1);

                        add_message(disk, sender, message, sender);
                    }
                }

                break;
            }

            case 6:
            {
                if (ID == -1)
                    printf("not logged in\n");
                else
                {
                    int receiver_ID;
                    printf("Enter other user's ID : ");
                    scanf("%d", &receiver_ID);

                    if (receiver_ID < 0 || no_of_users == 0 || receiver_ID >= no_of_users)
                        printf("no users found");
                    else
                    {
                        user *receiver = get_user_by_ID(users, receiver_ID);
                        
                        if (receiver->no_of_messages >= MESSAGE_SIZE)
                            printf("message limit reached\n");
                        else
                        {
                            char message[50];
                            printf("Enter message : ");
                            scanf("\n%48[^\n]", message + 1);

                            add_message(disk, get_user_by_ID(users, ID), message, receiver);
                        }
                    }
                }

                break;
            }

            case 7:
            {
                if (ID == -1 || no_of_users == 0 || ID >= no_of_users)
                    printf("not logged in\n");
                else
                {
                    ID = -1;
                    printf("logged out\n");
                }

                break;
            }

            case 8:
            {
                printf("exiting\n");
                choice = 8;
                break;
            }

            default:
            {
                printf("invalid choice\nexiting\n");
                choice = 8;
                break;
            }
        }
    }

    save_users(disk, users, no_of_users);
    fclose(disk);
    return 0;
}