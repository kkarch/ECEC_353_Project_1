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
    char private_user[USER_NAME_LEN];
    //from send_msg.c
    int flags;
    mqd_t mqd;
    flags = O_WRONLY;
    flags |= O_NONBLOCK;
    unsigned int priority;
    
    char message[2048]; //this is the message size specified in the server
    char pm_message[2048+USER_NAME_LEN];
    

    if (argc != 2) {
        printf ("Usage: %s user-name message\n", argv[0]);
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
                  printf ("Option B.\n");
                  printf("\nEnter Message: ");
                  dummy=getchar(); //clears the input buffer
                  fgets (message, sizeof(message), stdin);
                  //printf("%s\n", message);
                  
                  /* FIXME: Send message to server to be broadcast */
                  priority=0; 
                  /* Open the specified MQ for O_WRONLY operation */
                  mqd = mq_open ("/mads_and_kev_mq", flags);
                  if (mqd == (mqd_t) -1) {
                  perror ("mq_open");
                  exit (EXIT_FAILURE);
                              }
                  if (mq_send (mqd, message, strlen (message), priority) == -1) {
                  perror ("mq_send");
                  exit (EXIT_FAILURE);
                              }
                  printf("Broadcast saying '%s' sent!\n", message);


                break;

            case 'P':
                  printf ("Option P\n");
                  
                  printf("\nEnter Message: ");
                  dummy=getchar(); //clears the input buffer
                  fgets (message, sizeof(message), stdin);
                  //printf("%s\n", message);
                  strcat(pm_message,message);
                  
                  printf("\nEnter Recipient User Name: ");
                  //dummy=getchar(); //clears the input buffer
                  fgets (private_user, sizeof(private_user), stdin);
                  strcat(pm_message,private_user);//this will be sent to the server and it will know if the priority is not 0 then the priority is the length of the message before the user name
                  /* FIXME: Get name of private user and send the private 
                 * message to server to be sent to private user */
                  printf("%s", pm_message);
                  priority=strlen(message); 
                  /* Open the specified MQ for O_WRONLY operation */
                  mqd = mq_open ("/mads_and_kev_mq", flags);
                  if (mqd == (mqd_t) -1) {
                  perror ("mq_open");
                  exit (EXIT_FAILURE);
                              }
                  if (mq_send (mqd, pm_message, strlen (pm_message), priority) == -1) {
                  perror ("mq_send");
                  exit (EXIT_FAILURE);
                              }
                  printf("Private message saying %s sent to %s!\n", message, private_user);

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
