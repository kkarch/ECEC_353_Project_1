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

#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "msg_structure.h"

#include <signal.h>
#include <limits.h>
#include <setjmp.h>

#define FALSE 0
#define TRUE !FALSE
#define CTRLC  1337

static void custom_signal_handler (int); 
static sigjmp_buf env;

/* function called by signal handler */
void 
quit_server ()
{
    printf("\n CTRL+C Received. Shutting down message queue. \n");
    /* mq_unlink(mq_pathname) */
    if (mq_unlink ("/mads_and_kev_mq") == -1) {
        perror ("Client: mq_unlink");
        exit (1);
    }
}


int 
main (int argc, char **argv) 
{
    char private_user[USER_NAME_LEN];
    char message[2048]; //this is the message size specified in the server
    char pm_message[2048+USER_NAME_LEN];
    //establish signal handler
    signal (SIGINT, custom_signal_handler);
    signal (SIGQUIT, custom_signal_handler);


    int ret;
    ret = sigsetjmp (env, TRUE);
    switch (ret) {
        case 0:
            /* Returned from explicit sigsetjmp call. */
            break;

        case CTRLC:
            quit_server();
            exit (EXIT_SUCCESS);
    }

    //create message queue one time
    int flags;
    mode_t perms;
    mqd_t mqd;
    struct mq_attr attr, *attrp;
    unsigned int priority;
    void *buffer;
    ssize_t nr;

    /* Set the default message queue attributes. */
    attrp = NULL;
    attr.mq_maxmsg = 10;    /* Maximum number of messages on queue */
    attrp = &attr;
    attr.mq_msgsize = 2048; /* Maximum message size in bytes */
    attrp = &attr;
    flags = O_RDWR;         /* Create or open the queue for reading and writing */
    flags |= O_CREAT;

    perms = S_IRUSR | S_IWUSR;  /* rw------- permissions on the queue */

    mqd = mq_open ("/mads_and_kev_mq", flags, perms, attrp);
    if (mqd == (mqd_t)-1) {
        perror ("mq_open");
        exit (EXIT_FAILURE);
    }

    while (1) {
       /* FIXME: Server code here */
       //recieve message code
           /* Get the attributes of the MQ */
        if (mq_getattr (mqd, &attr) == -1) {
            perror ("mq_getattr");
            exit (EXIT_FAILURE);
        }
    
        /* Allocate local buffer to store the received message from the MQ */
        buffer = malloc (sizeof(attr.mq_msgsize));
        if (buffer == NULL) {
            perror ("malloc");
            exit (EXIT_FAILURE);
        }
    
        nr = mq_receive (mqd, buffer, attr.mq_msgsize, &priority);
        if (nr == -1) {
            perror ("mq_receive");
            exit (EXIT_FAILURE);
        }
    
        printf ("Read %ld bytes; priority = %u \n", (long) nr, priority);
        if (priority!=0){
         //split up message and username, priority is byte length of the message
        
        }
        if (write (STDOUT_FILENO, buffer, nr) == -1) {
            perror ("write");
            exit (EXIT_FAILURE);
        }
       //client tracking
       //notifications & more
       printf("Server running... \n");
       sleep(3);
    }
    exit (EXIT_SUCCESS);
}

static void
custom_signal_handler (int signalNumber)
{
    signal (SIGINT, custom_signal_handler);
    signal (SIGQUIT, custom_signal_handler);
    siglongjmp(env,CTRLC);
}
