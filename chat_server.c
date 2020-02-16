/* Skeleton code for the server side code. 
 * 
 * Compile as follows: gcc -o chat_server chat_server.c -std=c99 -Wall -lrt
 *
 * Author: Naga Kandasamy
 * Date created: January 28, 2020
 *
 * Student/team name: Madeline Cook, Kevin Karch
 * Date created: 2/7/2020 
 *
 */
#define _POSIX_C_SOURCE 1
//#define _POSIX_C_SOURCE 2
#define USER_LIMIT 2
#define SERVER_WKK "/MCKK_Server"
#define SERVER_PREFIX "/MCKK_%s"

#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "msg_structure.h"
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <setjmp.h>
#include <pthread.h>

#define FALSE 0
#define TRUE !FALSE
#define CTRLC 1337

static void custom_signal_handler(int);
static sigjmp_buf env;
static char client_name[USER_NAME_LEN];

/* function called by signal handler */
void quit_server()
{
    printf("\n CTRL+C Received. Shutting down message queue. \n");
    /* mq_unlink(mq_pathname) */
    if (mq_unlink(SERVER_WKK) == -1)
    {
        perror("Client: mq_unlink");
        exit(1);
    }
}

int main(int argc, char **argv)
{
    signal(SIGINT, custom_signal_handler);
    signal(SIGQUIT, custom_signal_handler);


    struct client_msg msg;
    struct server_msg smsg;


    /* Server MQ */
    int flags;
    mode_t perms;
    mqd_t mqd;
    struct mq_attr attr, *attrp;
    ssize_t nr;
    attrp = NULL;
    attr.mq_maxmsg = 10; /* Maximum number of messages on queue */
    attrp = &attr;
    attr.mq_msgsize = sizeof(msg); /* Maximum message size in bytes */
    flags = O_RDWR;                /* Create or open the queue for reading and writing */
    flags |= O_CREAT;

    /* Client MQ */
    int cflags;
    mqd_t cmqd;
    cflags = O_WRONLY;
    cflags |= O_NONBLOCK;
    //unsigned int priority;

    perms = S_IRUSR | S_IWUSR; /* rw------- permissions on the queue */

    int ret;
    ret = sigsetjmp(env, TRUE);
    switch (ret)
    {
    case 0:
        /* Returned from explicit sigsetjmp call. */
        break;

    case CTRLC:
        quit_server();
        exit(EXIT_SUCCESS);
    }

    mqd = mq_open(SERVER_WKK, flags, perms, attrp);
    if (mqd == (mqd_t)-1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    char users[USER_LIMIT][USER_NAME_LEN];
    int count, i, found_match = 0;
    while (1)
    {
        nr = mq_receive(mqd, (char *)&msg, sizeof(msg) + 1, 0);
        if (nr == -1)
        {
            perror("mq_receive");
            exit(EXIT_FAILURE);
        }
        switch (msg.control)
        {
        case 0:
            /* Check-in Code */
            if (count < USER_LIMIT)
            {
                strcpy(users[count], msg.user_name);
                count++;
            }
            else
            {
                printf("Checking for Space\n");
                for (i = 0; i <= USER_LIMIT; i++)
                {
                    if (!strcmp(users[i], "Empty"))
                    {
                        strcpy(users[i], msg.user_name);
                        break;
                    }
                    else if (i == USER_LIMIT)
                    {
                        printf("!!!Server Full!!!\n");
                        printf("%d\n", msg.client_pid);
                        kill(msg.client_pid, SIGINT);
                    }
                }
            }

        case 1:

            if (msg.broadcast == 1)
            {
                printf("\nUser %s would like to broadcast\n", msg.user_name);
                strcpy(smsg.msg, msg.msg);
                strcpy(smsg.sender_name, msg.user_name);
                for (i = 0; i < count; i++)
                {
                    if (strcmp(users[i], "Empty"))
                    {
                        printf("\nOpening %s\n", users[i]);
                        cmqd = mq_open(users[i], cflags);
                        if (cmqd == (mqd_t)-1)
                        {
                            perror("mq_open");
                            exit(EXIT_FAILURE);
                        }
                        printf("Sending %s to %s\n", smsg.msg, users[i]);
                        if (mq_send(cmqd, (char *)&smsg, sizeof(smsg), 0) == -1)
                        {
                            perror("mq_send");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }
            else if (msg.broadcast == 0)
            {
                found_match = 0;
                snprintf(client_name, MESSAGE_LEN, SERVER_PREFIX, (char *)msg.priv_user_name);
                printf("\nUser %s would like to talk to %s\n", msg.user_name, client_name);
                strcpy(smsg.msg, msg.msg);
                strcpy(smsg.sender_name, msg.user_name);
                for (i = 0; i < count; i++)
                {
                    //printf("Comparing %s:%s\n",users[i],client_name);
                    if (strcmp(users[i], client_name) == 0)
                    {
                        printf("Opening %s\n", client_name);
                        cmqd = mq_open(client_name, cflags);
                        if (cmqd == (mqd_t)-1)
                        {
                            perror("mq_open");
                            exit(EXIT_FAILURE);
                        }
                        printf("Sending %s to %s\n", smsg.msg, client_name);
                        if (mq_send(cmqd, (char *)&smsg, sizeof(smsg), 0) == -1)
                        {
                            perror("mq_send");
                            exit(EXIT_FAILURE);
                        }
                        found_match = 1;
                        break;
                    }
                }
                if (!found_match)
                {
                    cmqd = mq_open(msg.user_name, cflags);
                    if (cmqd == (mqd_t)-1)
                    {
                        perror("mq_open");
                        exit(EXIT_FAILURE);
                    }
                    strcpy(smsg.msg, "Error: No User With that ID");
                    strcpy(smsg.sender_name, "Server");
                    if (mq_send(cmqd, (char *)&smsg, sizeof(smsg), 0) == -1)
                    {
                        perror("mq_send");
                        exit(EXIT_FAILURE);
                    }
                    printf("User %s not found\n", client_name);
                }
            }
            break;

        case 2:
            /* Exiting Code */
            printf("User: %s has left the chat\n", msg.user_name);
            for (i = 0; i < USER_LIMIT; i++)
            {
                if (!strcmp(users[i], msg.user_name))
                {
                    strcpy(users[i], "Empty");
                    break;
                }
            }
            break;
        case 3:
            /* Hidden Testing Code */
            for (int j = 0; j < count; j++)
            {
                if (strcmp(users[j], ""))
                {
                    printf("User%d: %s\n", j, users[j]);
                }
            }
            break;
        default:
            break;
        }

        //printf("%s\n",msg.user_name);
        //printf
        //printf("Server running... \n");
        //sleep(3);
    }
    exit(EXIT_SUCCESS);
}

static void
custom_signal_handler(int signalNumber)
{
    signal(SIGINT, custom_signal_handler);
    signal(SIGQUIT, custom_signal_handler);
    siglongjmp(env, CTRLC);
}
