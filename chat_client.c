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
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "msg_structure.h"

void 
print_main_menu (void)
{
    printf ("\n'B'roadcast message\n");
    printf ("'P'rivate message\n");
    printf ("'E'xit\n");
    return;
}

int 
main (int argc, char **argv)
{
    char user_name[USER_NAME_LEN];
    char private_name[USER_NAME_LEN];//recipient of private messages //we could add this to the back of the private message and then handle it on the server side
    //from send_msg.c
    int flags;
    mqd_t mqd;
    flags = O_WRONLY;
    flags |= O_NONBLOCK;
    unsigned int priority; //can be used to indicate if the message is for everyone or for a private user

    if (argc != 2) {
        printf ("Usage: %s user-name\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    strcpy (user_name, argv[1]); /* Get the client user name */

    /* FIXME: Connect to server */
    printf ("User %s connecting to server\n", user_name);

    /* Operational menu for client */
    char option, dummy;
    while (1) {
        print_main_menu ();
        option = getchar ();

        switch (option) {
            case 'B':
               /* FIXME: Send message to server to be broadcast */
               priority=10; 
               /* Open the specified MQ for O_WRONLY operation */
                mqd = mq_open ("/mads_and_kev_mq", flags);
                if (mqd == (mqd_t) -1) {
                    perror ("mq_open");
                    exit (EXIT_FAILURE);
                }
                if (mq_send (mqd, argv[2], strlen (argv[2]), priority) == -1) { //i think the message is argv 2 bc argv 1 is the user name
                    perror ("mq_send");
                    exit (EXIT_FAILURE);
                }
                break;

            case 'P':
                /* FIXME: Get name of private user and send the private 
                 * message to server to be sent to private user */
                 priority=10;
                mqd = mq_open ("/mads_and_kev_mq", flags);
                if (mqd == (mqd_t) -1) {
                    perror ("mq_open");
                    exit (EXIT_FAILURE);
                }
                if (mq_send (mqd, argv[2], strlen (argv[2]), priority) == -1) { //i think the message is argv 2 bc argv 1 is the user name
                    perror ("mq_send");
                    exit (EXIT_FAILURE);
                }
                break;

            case 'E':
                printf ("Chat client exiting\n");
                /* FIXME: Send message to server that we are exiting */
                exit (EXIT_SUCCESS);

            default:
                printf ("Unknown option\n");
                break;
                
        }
        /* Read dummy character to consume the \n left behind in STDIN */
        dummy = getchar ();
    }
         
    exit (EXIT_SUCCESS);
}
