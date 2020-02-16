/* Skeleton code for the client side code. 
 *
 * Compile as follows: gcc -o chat_client chat_client.c -std=c99 -Wall -lrt
 *
 * Author: Naga Kandsamy
 * Date created: January 28, 2020
 * Date modified:
 *
 * Student/team name: Madeline Cook, Kevin Karch
 * Date created: 2/7/2020 
 *
*/

#define _POSIX_C_SOURCE 2 // For getopt()

#include <mqueue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "msg_structure.h"

pid_t getpid(void);

void print_main_menu(void)
{
    printf("\n'B'roadcast message\n");
    printf("'P'rivate message\n");
    printf("'E'xit\n");
    return;
}

int main(int argc, char **argv)
{
    char user_name[USER_NAME_LEN];
    char private_user[USER_NAME_LEN];
    //from send_msg.c
    int flags;
    mqd_t mqd;
    flags = O_WRONLY;
    flags |= O_NONBLOCK;
    unsigned int priority;

    char message[MESSAGE_LEN]; //this is the message size specified in the server
    char pm_message[MESSAGE_LEN];

    struct client_msg msg;

    if (argc != 2)
    {
        printf("Usage: %s user-name message\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strcpy(user_name, argv[1]); /* Get the client user name */

    /* Connect to server */
    printf("User %s connecting to server\n", user_name);

    mqd = mq_open("/MCKK_Server", flags);
    if (mqd == (mqd_t)-1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    /* Init Structure */
    msg.broadcast = 0;
    msg.client_pid = getpid();
    msg.control = 0;
    strcpy(msg.msg, "Empty");
    strcpy(msg.priv_user_name, "None");
    strcpy(msg.user_name, user_name);

    /* Send request to connect */
    if (mq_send(mqd, (char *)&msg, sizeof(msg), 0) == -1)
    {
        perror("mq_send");
        exit(EXIT_FAILURE);
    }

    /* Operational menu for client */
    char option, dummy;
    while (1)
    {
        print_main_menu();
        option = getchar();

        switch (option)
        {
        case 'B':
            printf("Option B.\n");
            printf("\nEnter Message: ");
            dummy = getchar(); //clears the input buffer
            fgets(message, sizeof(message), stdin);
            //printf("%s\n", message);

            /* Update Structure*/
            priority = 0;
            msg.broadcast = 1;
            msg.control = 1;
            strcpy(msg.msg, message);

            /* Full Send */
            if (mq_send(mqd, (char *)&msg, sizeof(msg), priority) == -1)
            {
                perror("mq_send");
                exit(EXIT_FAILURE);
            }
            printf("Broadcast saying '%s' sent!\n", message);

            break;

        case 'P':
            msg.control = 1; // Normal Op
            priority = 0;
            msg.broadcast = 0;

            printf("Option P\n");
            printf("\nEnter Message: ");

            dummy = getchar(); //clears the input buffer
            fgets(message, sizeof(message), stdin);
            strcpy(msg.msg, message);

            printf("\nEnter Recipient User Name: ");

            fgets(private_user, sizeof(private_user), stdin);
            strcpy(msg.priv_user_name, private_user); 

            if (mq_send(mqd, (char *)&msg, sizeof(msg), priority) == -1)
            {
                perror("mq_send");
                exit(EXIT_FAILURE);
            }
            printf("Private message saying %s sent to %s!\n", message, private_user);

            break;

        case 'E':
            printf("Chat client exiting\n");
            /* FIXME: Send message to server that we are exiting */
            msg.control = 2;

            priority = 0;

            if (mq_send(mqd, (char *)&msg, sizeof(msg), priority) == -1)
            {
                perror("mq_send");
                exit(EXIT_FAILURE);
            }
            
            exit(EXIT_SUCCESS);

        case 'T':
            msg.control = 3;
            dummy = getchar();
            printf("\nTesting Structure\n");
            //msg = (struct msgbuf*)malloc(sizeof(struct client_msg)+1+4096);
            priority = 0;
            if (mq_send(mqd, (char *)&msg, sizeof(msg), 0) == -1)
            {
                perror("mq_send");
                exit(EXIT_FAILURE);
            }
            break;
        default:
            printf("Unknown option\n");
            break;
        }
        /* Read dummy character to consume the \n left behind in STDIN */
        //dummy = getchar ();
    }

    exit(EXIT_SUCCESS);
}
