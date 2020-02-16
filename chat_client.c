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
#include <pthread.h>

pid_t getpid(void);

static void setup_notification (mqd_t *mqdp);

static void 
thread_func (union sigval sv)
{
    ssize_t nr;
    mqd_t *mqdp;
    void *mbuffer;
    struct server_msg *buffer;
    struct mq_attr attr;

    mqdp = sv.sival_ptr;

    if (mq_getattr (*mqdp, &attr) == -1) {
        perror ("mq_getattr");
        exit (EXIT_FAILURE);
    }

    mbuffer = malloc (attr.mq_msgsize);
    if (mbuffer == NULL) {
        perror ("malloc");
        exit (EXIT_FAILURE);
    }


    /* Reenable notification */
    setup_notification (mqdp);

    /* Drain the queue empty */
    while ((nr = mq_receive (*mqdp, mbuffer, attr.mq_msgsize, NULL)) >= 0)
        buffer = (struct server_msg *) mbuffer;
        printf ("%s: %s\n",buffer->sender_name,buffer->msg);

    free (buffer);

    return;
}

static void 
setup_notification (mqd_t *mqdp)
{
    struct sigevent sev;

    sev.sigev_notify = SIGEV_THREAD;            /* Notify via thread */
    sev.sigev_notify_function = thread_func;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_value.sival_ptr = mqdp;           /* Argument to thread_func() */

    if (mq_notify (*mqdp, &sev) == -1) {
        perror ("mq_notify");
        exit (EXIT_FAILURE);
    }

    return;
}

void print_main_menu(void)
{
    printf("\n'B'roadcast message\n");
    printf("'P'rivate message\n");
    printf("'E'xit\n");
    return;
}

void quit_client(const char* client)
{
    printf("\nConnection Error: Shutting down message queue. \n");
    /* mq_unlink(mq_pathname) */
    if (mq_unlink(client) == -1)
    {
        perror("Client: mq_unlink");
        exit(1);
    }
}

int main(int argc, char **argv)
{
    struct client_msg msg;
    struct server_msg smsg;

    char user_name[USER_NAME_LEN];
    char private_user[USER_NAME_LEN];
    char message[MESSAGE_LEN]; //this is the message size specified in the server
    //char pm_message[MESSAGE_LEN];
    char client_name[MESSAGE_LEN];

    /* Server MQ */
    int flags;
    mqd_t mqd;
    flags = O_WRONLY;
    flags |= O_NONBLOCK;
    unsigned int priority;

    /* Client MQ */
    //int nr;
    int cflags;
    mqd_t cmqd;
    struct mq_attr attr;
    mode_t perms;
    /* Set the default message queue attributes. */
    attr.mq_maxmsg = 10;    /* Maximum number of messages on queue */
    attr.mq_msgsize = sizeof(smsg); /* Maximum message size in bytes */
    cflags = O_RDWR;         
    cflags |= O_CREAT;
    cflags |= O_NONBLOCK;
    perms = S_IRUSR | S_IWUSR; /* rw------- permissions on the queue */



    if (argc != 2)
    {
        printf("Usage: %s user-name message\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strcpy(user_name, argv[1]); /* Get the client user name */

    /* Connect to server */
    printf("User %s connecting to server\n", user_name);



    snprintf (client_name, MESSAGE_LEN, "/MCKK_%s", (char*) user_name);
    cmqd = mq_open(client_name, cflags, perms, &attr);
    if (cmqd == (mqd_t)-1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    setup_notification(&cmqd);

    mqd = mq_open("/MCKK_Server", flags);
    if (mqd == (mqd_t)-1)
    {
        perror("mq_open");
        quit_client(client_name);
        exit(EXIT_FAILURE);
    }

    /* Init Structure */
    msg.broadcast = 3;
    msg.client_pid = getpid();
    msg.control = 0;
    strcpy(msg.msg, "Empty");
    strcpy(msg.priv_user_name, "None");
    strcpy(msg.user_name, client_name);

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

        //mq_receive(cmqd, (char *)&smsg, sizeof(smsg) + 1, 0);
        //if(!strcmp(smsg.msg,"Hello")){printf("%s\n",smsg.msg);}

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
            if(private_user[strlen(private_user)-1] == '\n'){
                private_user[strlen(private_user)-1] = '\0';
            }
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

            if (mq_unlink(msg.user_name) == -1)
            {
                perror("Client: mq_unlink");
                exit(1);
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

