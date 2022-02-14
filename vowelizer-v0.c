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

char UDPmessagein[MAX_MESSAGE_LENGTH];
int vindex = 0;

/* This is a signal handler to do graceful exit if needed */
void catcher( int sig )
{
    close(childsockfd);
    exit(0);
}
  
void devowelTCP(char* message);
void devowelUDP(char* message);
void envowel(char* message);  

/* Main program for server */
int main()
  {
    struct sockaddr_in TCPserver, UDPserver, client;
    static struct sigaction act;
    char TCPmessagein[MAX_MESSAGE_LENGTH];
    char TCPmessageout[MAX_MESSAGE_LENGTH];
	char UDPmessageout[MAX_MESSAGE_LENGTH];
    int parentsockfd, UDPsocket;
    int i, len, bytes, answer;
    pid_t pid;
	
	len = sizeof(TCPserver);

    /* Set up a signal handler to catch some weird termination conditions. */
    act.sa_handler = catcher;
    sigfillset(&(act.sa_mask));
    sigaction(SIGPIPE, &act, NULL);

    /* Initialize server sockaddr structure */
    memset(&TCPserver, 0, sizeof(TCPserver));
    TCPserver.sin_family = AF_INET;
    TCPserver.sin_port = htons(TCP_PORT);
    TCPserver.sin_addr.s_addr = htonl(INADDR_ANY);
	
	/* Initialize server sockaddr structure */
    memset((char*) &UDPserver, 0, sizeof(UDPserver));
    UDPserver.sin_family = AF_INET;
    UDPserver.sin_port = htons(UDP_PORT);
    UDPserver.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//fprintf(stderr, "DEBUG\n");

    /* create a transport-level endpoint using TCP */
    /* set up the transport-level end point to use TCP */
    if((parentsockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
		fprintf(stderr, "ERROR: socket() call failed!\n");
		exit(1);
    }
	
	if((UDPsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
		fprintf(stderr, "ERROR: socket() call failed!\n");
		exit(1);
    }

    /* bind a specific address and port to the end point */
    if(bind(parentsockfd, (struct sockaddr *) &TCPserver, sizeof(struct sockaddr_in)) == -1)
    {
		fprintf(stderr, "ERROR: bind() call failed!\n");
		exit(1);
    }
	
	/* bind a UDP */
    if(bind(UDPsocket, (struct sockaddr *) &UDPserver, sizeof(UDPserver)) == -1)
    {
		fprintf(stderr, "ERROR: UDP bind() call failed!\n");
		exit(1);
    }

    /* start listening for incoming connections from clients */
    if(listen(parentsockfd, 5) == -1)
    {
		fprintf(stderr, "ERROR: listen() call failed!\n");
		exit(1);
    }

    /* initialize message strings just to be safe (null-terminated) */
    bzero(TCPmessagein, MAX_MESSAGE_LENGTH);
	bzero(UDPmessagein, MAX_MESSAGE_LENGTH);
    bzero(TCPmessageout, MAX_MESSAGE_LENGTH);
	bzero(UDPmessageout, MAX_MESSAGE_LENGTH);

    fprintf(stderr, "Hello there! I am the vowelizer server!!\n");
    fprintf(stderr, "I am running on TCP port %d and UDP port %d...\n\n", TCP_PORT, UDP_PORT);

    /* Main loop: server loops forever listening for requests */
    for( ; ; )
    {
		bzero(TCPmessagein, MAX_MESSAGE_LENGTH);
		bzero(UDPmessagein, MAX_MESSAGE_LENGTH);
		bzero(TCPmessageout, MAX_MESSAGE_LENGTH);
		bzero(UDPmessageout, MAX_MESSAGE_LENGTH);
		
		/* accept a connection */
		if((childsockfd = accept(parentsockfd, NULL, NULL)) == -1)
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
	    while(recv(childsockfd, TCPmessagein, MAX_MESSAGE_LENGTH, 0) > 0)
	    {
			if(recvfrom(UDPsocket, UDPmessagein, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *) &client, &len) < 0)
			{
				printf("ERROR: UDP recvfrom() error!\n");
				exit(1);
			}
			/* print out the received message */
			printf("Child process received word: %s\n", TCPmessagein);
			printf("That word has %d characters!\n", strlen(TCPmessagein));
			devowelTCP(TCPmessagein);
			devowelUDP(UDPmessagein);
			printf("The devowled word: %s\n", TCPmessagein);
			printf("The vowled word: %s\n", UDPmessagein);

			/* create the outgoing message (as an ASCII string) */
			sprintf(TCPmessageout, "%s\n", TCPmessagein);
			sprintf(UDPmessageout, "%s\n", UDPmessagein);

			#ifdef DEBUG
			printf("Sent %d bytes '%s' to client using TCP\n", strlen(TCPmessageout),TCPmessageout);
			printf("Sent %d bytes '%s' to client using UDP\n", strlen(UDPmessageout), UDPmessageout);
			#endif

			/* send the result message back to the client */
			send(childsockfd, TCPmessageout, strlen(TCPmessageout), 0);
			
			sendto(UDPsocket, UDPmessageout, strlen(UDPmessageout), 0, (struct sockaddr *) &client, len);

			/* clear out message strings again to be safe */
			bzero(TCPmessagein, MAX_MESSAGE_LENGTH);
			bzero(TCPmessageout, MAX_MESSAGE_LENGTH);
			bzero(UDPmessageout, MAX_MESSAGE_LENGTH);
			bzero(UDPmessagein, MAX_MESSAGE_LENGTH);
	    }

			/* when client is no longer sending information to us, */
			/* the socket can be closed and the child process terminated */
			close(childsockfd);
			close(UDPsocket);
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

void devowelTCP(char* message)
{
	char* iter = message;
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

void devowelUDP(char* message)
{
	char* it = message;
	for(int i = 0; i < strlen(message) && it != NULL; i++)
	{
		if((*it == 'a') || (*it == 'e') || (*it == 'i') || (*it == 'o') || (*it == 'u'))
			UDPmessagein[vindex] = *it;
		else
			UDPmessagein[vindex] = ' ';
		vindex++;
		it++;	
	}
}

void envowel(char* message)
{

}	