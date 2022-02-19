#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>

/* Some generic error handling stuff */
extern int errno;
void perror(const char *s);

// IP address and port of server
#define SERVER_IP "127.0.0.1"
#define TCPPORTNUM 44145
#define UDPPORTNUM 44145

/* Manifest constants used by client program */
#define MAX_HOSTNAME_LENGTH 64
#define MAX_WORD_LENGTH 100

/* Menu selections */
#define ALLDONE 0
#define DEVOWEL 1
#define ENVOWEL 2

/* Prompt the user to enter a word */
void printmenu()
{
    printf("\n\n\n");
    printf("Please choose from the following selections:\n");
    printf("  1 - Devowel a message\n");
	printf("  2 - Envowel a message\n");
    printf("  0 - Exit program\n");
    printf("Your desired menu selection? ");
}

/* Main program of client */
int main()
  {
    int sockfd, sockfd2;
    char c, c1;
    struct sockaddr_in server, server1;
    struct hostent *hp;
    char hostname[MAX_HOSTNAME_LENGTH];
    char messageTCP[MAX_WORD_LENGTH];
	char messageUDP[MAX_WORD_LENGTH];
    char messageback[MAX_WORD_LENGTH];
	char messagebackUDP[MAX_WORD_LENGTH];
    int choice, len, bytes, len1;
	int UDPsocket;
	
	len = sizeof(server1);

    /* Initialization of server sockaddr data structure */
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(TCPPORTNUM);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if((UDPsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
		fprintf(stderr, "ERROR: socket() call failed!\n");
		exit(1);
    }
	
	memset((char*) &server1, 0, sizeof(server1));
    server1.sin_family = AF_INET;
    server1.sin_port = htons(UDPPORTNUM);
    server1.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(inet_pton(AF_INET, SERVER_IP, &server1.sin_addr)==0)
	{
		printf("inet_pton() failed!\n");
		exit(1);
	}
	fprintf(stderr, "DEBUG\n");

    /* hard code the IP address so you don't need hostname resolver */
    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    /* create the client socket for its transport-level end point */
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1 )
    {
		fprintf(stderr, "ERROR: socket() call failed!\n");
		exit(1);
    }

    /* connect the socket to the server's address using TCP */
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1 )
    {
		fprintf(stderr, "ERROR: connect() call failed!\n");
		perror(NULL);
		exit(1);
    }

    /* Print welcome banner */
    printf("Welcome! I am the TCP version of the word length client!!\n");
    printmenu();
    scanf("%d", &choice);

    /* main loop: read a word, send to server, and print answer received */
    while(choice != ALLDONE )
    {
		bzero(messageTCP, MAX_WORD_LENGTH);
		bzero(messageUDP, MAX_WORD_LENGTH);
		bzero(messageback, MAX_WORD_LENGTH);
		bzero(messagebackUDP, MAX_WORD_LENGTH);
		if(choice == DEVOWEL )
	    {
			/* get rid of newline after the (integer) menu choice given */
			c = getchar();

			/* prompt user for the input */
			printf("Enter your message to devowel: ");
			len = 0;
			while((c = getchar()) != '\n')
			{
				messageTCP[len] = c;
				len++;
			}
			/* make sure the message is null-terminated in C */
			messageTCP[len] = '\0';

			/* send it to the server via the socket */
			send(sockfd, messageTCP, len, 0);
			
			if(sendto(UDPsocket, messageTCP, strlen(messageTCP), 0, (struct sockaddr *)&server1, sizeof(server1)) == -1)
			{
				printf("ERROR: sendto() failed!\n");
				exit(1);
			}

			/* see what the server sends back */
			// &&
			// recvfrom(UDPsocket, messagebackUDP, MAX_WORD_LENGTH, 0, (struct sockaddr *)&server1, &len)
			if(((bytes = recv(sockfd, messageback, len, 0)) > 0))
			{
				/* make sure the message is null-terminated in C */
				messageback[bytes] = '\0';
				printf("Server sent %ld bytes of non-vowels on TCP: '%s'\n", strlen(messageback), messageback);
			}
			else
			{
				/* an error condition if the server dies unexpectedly */
				printf("ERROR: Server failed!\n");
				close(sockfd);
				exit(1);
			}
			
			if( recvfrom(UDPsocket, messagebackUDP, MAX_WORD_LENGTH, 0, (struct sockaddr *)&server1, &len) > 0)
			{
				messagebackUDP[bytes] = '\0';
				printf("Server sent %ld bytes of     vowels on UDP: '%s'", strlen(messagebackUDP), messagebackUDP);
			}
			else
			{
				/* an error condition if the server dies unexpectedly */
				printf("ERROR: Server failed!\n");
				close(sockfd);
				exit(1);
			}
	  }
	  else if(choice == ENVOWEL )
	  {
		  /* get rid of newline after the (integer) menu choice given */
			c = getchar();

			/* prompt user for the input */
			printf("Enter non-vowel part of message to envowel: ");
			len = 0;
			while((c = getchar()) != '\n')
			{
				messageTCP[len] = c;
				len++;
			}
			/* make sure the message is null-terminated in C */
			messageTCP[len] = '\0';

			/* send it to the server via the socket */
			send(sockfd, messageTCP, len, 0);
			
			c1 = getchar();

			/* prompt user for the input */
			printf("Enter vowel part of message to envowel: ");
			len1 = 0;
			while((c1 = getchar()) != '\n')
			{
				messageUDP[len1] = c1;
				len1++;
			}
			/* make sure the message is null-terminated in C */
			messageUDP[len1] = '\0';
			
			if(sendto(UDPsocket, messageUDP, strlen(messageUDP), 0, (struct sockaddr *)&server1, sizeof(server1)) == -1)
			{
				printf("ERROR: sendto() failed!\n");
				exit(1);
			}

			/* see what the server sends back */
			// &&
			// recvfrom(UDPsocket, messagebackUDP, MAX_WORD_LENGTH, 0, (struct sockaddr *)&server1, &len)
			if(((bytes = recv(sockfd, messageback, len, 0)) > 0))
			{
				/* make sure the message is null-terminated in C */
				messageback[bytes] = '\0';
				printf("Server sent %ld bytes of non-vowels on TCP: '%s'\n", strlen(messageback), messageback);
			}
			else
			{
				/* an error condition if the server dies unexpectedly */
				printf("ERROR: Server failed!\n");
				close(sockfd);
				exit(1);
			}
			
			if( recvfrom(UDPsocket, messagebackUDP, MAX_WORD_LENGTH, 0, (struct sockaddr *)&server1, &len1) > 0)
			{
				messagebackUDP[bytes] = '\0';
				printf("Server sent %ld bytes of     vowels on UDP: '%s'", strlen(messagebackUDP), messagebackUDP);
			}
			else
			{
				/* an error condition if the server dies unexpectedly */
				printf("ERROR: Server failed!\n");
				close(sockfd);
				exit(1);
			}
		  
	  }
	  else printf("ERROR: Invalid menu selection. Please try again.\n");

	  printmenu();
	  scanf("%d", &choice);
    }

    /* Program all done, so clean up and exit the client */
    close(sockfd);
    exit(0);
  }