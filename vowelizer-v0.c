#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

/* Global manifest constants */
#define MAX_MESSAGE_LENGTH 100
#define TCP_PORT 44144         
#define UDP_PORT 44144
#define SIMPLE 1

/* Optional verbose debugging output */
#define DEBUG 1 

/* Global variable */
int childsockfd;

/* This is a signal handler to do graceful exit if needed */
void catcher( int sig )
{
    close(childsockfd);
    exit(0);
}
  
void devowel(char* message);
void envowel(char* message);  

/* Main program for server */
int main()
  {
    struct sockaddr_in server;
    static struct sigaction act;
    char messagein[MAX_MESSAGE_LENGTH];
    char TCPmessageout[MAX_MESSAGE_LENGTH];
	char UDPmessageout[MAX_MESSAGE_LENGTH];
    int parentsockfd;
    int i, len, bytes, answer;
    pid_t pid;

    /* Set up a signal handler to catch some weird termination conditions. */
    act.sa_handler = catcher;
    sigfillset(&(act.sa_mask));
    sigaction(SIGPIPE, &act, NULL);

    /* Initialize server sockaddr structure */
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(TCP_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    /* create a transport-level endpoint using TCP */
    /* set up the transport-level end point to use TCP */
    if((parentsockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1 )
    {
		fprintf(stderr, "wordlen-TCPserver: socket() call failed!\n");
		exit(1);
    }

    /* bind a specific address and port to the end point */
    if( bind(parentsockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in) ) == -1 )
    {
		fprintf(stderr, "ERROR: bind() call failed!\n");
		exit(1);
    }

    /* start listening for incoming connections from clients */
    if( listen(parentsockfd, 5) == -1 )
    {
		fprintf(stderr, "ERROR: listen() call failed!\n");
		exit(1);
    }

    /* initialize message strings just to be safe (null-terminated) */
    bzero(messagein, MAX_MESSAGE_LENGTH);
    bzero(TCPmessageout, MAX_MESSAGE_LENGTH);

    fprintf(stderr, "Hello there! I am the vowelizer server!!\n");
    fprintf(stderr, "I am running on TCP port %d and UDP port %d...\n\n", TCP_PORT, UDP_PORT);

    /* Main loop: server loops forever listening for requests */
    for( ; ; )
    {
		/* accept a connection */
		if( (childsockfd = accept(parentsockfd, NULL, NULL)) == -1 )
		{
			fprintf(stderr, "ERROR: accept() call failed!\n");
			exit(1);
		}

		/* try to create a child process to deal with this new client */
		pid = fork();

		/* use process id (pid) returned by fork to decide what to do next */
		if(pid < 0)
		{
			fprintf(stderr, "ERROR: fork() call failed! OMG!!\n");
			exit(1);
		}
		else if(pid == 0)
		{
			/* this is the child process doing this part */

			/* don't need the parent listener socket that was inherited */
			close(parentsockfd);

			 /* obtain the message from this client */
	    while(recv(childsockfd, messagein, MAX_MESSAGE_LENGTH, 0) > 0 )
	    {
			/* print out the received message */
			printf("Child process received word: %s\n", messagein);
			
			printf("That word has %d characters!\n", strlen(messagein));
			devowel(messagein);
			printf("The devowled word: %s\n", messagein);

			/* create the outgoing message (as an ASCII string) */
			sprintf(TCPmessageout, "%s\n", messagein);

			#ifdef DEBUG
			printf("Child about to send message: %s\n", TCPmessageout);
			#endif

			/* send the result message back to the client */
			send(childsockfd, TCPmessageout, strlen(TCPmessageout), 0);

			/* clear out message strings again to be safe */
			bzero(messagein, MAX_MESSAGE_LENGTH);
			bzero(TCPmessageout, MAX_MESSAGE_LENGTH);
	    }

			/* when client is no longer sending information to us, */
			/* the socket can be closed and the child process terminated */
			close(childsockfd);
			exit(0);
		} /* end of then part for child */
		else
		 {
	    /* the parent process is the one doing the "else" part */
	    fprintf(stderr, "Created child process %d to handle that client\n", pid);
	    fprintf(stderr, "Parent going back to job of listening...\n\n");

	    /* parent doesn't need the childsockfd */
	    close(childsockfd);
	  }
    }
}

void devowel(char* message)
{
	char* iter = message;
	for(int i = 0; i < strlen(message); i++)
	{
		
		iter = strchr(message, 'a');
		while(iter != NULL)
		{
			strncpy(iter, " ", 1);
			iter = strchr(message, 'a');
		}
		
		iter = strchr(message, 'e');
		while(iter != NULL)
		{
			strncpy(iter, " ", 1);
			iter = strchr(message, 'e');
		}
		
		iter = strchr(message, 'i');
		while(iter != NULL)
		{
			strncpy(iter, " ", 1);
			iter = strchr(message, 'i');
		}
		
		iter = strchr(message, 'o');
		while(iter != NULL)
		{
			strncpy(iter, " ", 1);
			iter = strchr(message, 'o');
		}
		
		iter = strchr(message, 'u');
		while(iter != NULL)
		{
			strncpy(iter, " ", 1);
			iter = strchr(message, 'u');
		}
			
	}
	
}

void envowel(char* message)
{

}	