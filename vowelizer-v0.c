#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

// Global manifest constants for CONFIGURATION
#define MAX_MESSAGE_LENGTH 100
#define TCP_PORT 44145
#define UDP_PORT 44145
#define SMPL 1            /////////////// SIMPLE FLAG FOR METHOD USE
#define DEVOWEL "DEVOWEL"
#define DEBUG 1 

int childsockfd;
char UDPmessagein[MAX_MESSAGE_LENGTH];
char TCPmessagein[MAX_MESSAGE_LENGTH];
char TCPmessageout[MAX_MESSAGE_LENGTH];
char UDPmessageout[MAX_MESSAGE_LENGTH];
int vindex = 0;
char advancedMessage[MAX_MESSAGE_LENGTH];
char advancedMessageUDP[MAX_MESSAGE_LENGTH];

// This is a signal handler to do graceful exit if needed 
void catcher( int sig )
{
    close(childsockfd);
    exit(0);
}
void devowel(char* message);
void envowel(char* nonvowels, char* vowels);  

// Main program for server 
int main()
  {
    struct sockaddr_in TCPserver, UDPserver, client;
    static struct sigaction act;
    int parentsockfd, UDPsocket;
    int i, len, bytes, answer;
    pid_t pid;
	len = sizeof(TCPserver);

    // Set up a signal handler to catch some weird termination conditions. */
    act.sa_handler = catcher;
    sigfillset(&(act.sa_mask));
    sigaction(SIGPIPE, &act, NULL);

    // Initialize server sockaddr structure  TCP
    memset(&TCPserver, 0, sizeof(TCPserver));
    TCPserver.sin_family = AF_INET;
    TCPserver.sin_port = htons(TCP_PORT);
    TCPserver.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// Initialize server sockaddr structure UDP
    memset((char*) &UDPserver, 0, sizeof(UDPserver));
    UDPserver.sin_family = AF_INET;
    UDPserver.sin_port = htons(UDP_PORT);
    UDPserver.sin_addr.s_addr = htonl(INADDR_ANY);

    // create a transport-level endpoint using TCP 
    // set up the transport-level end point to use TCP 
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
    // bind a specific address and port to the end point 
    if(bind(parentsockfd, (struct sockaddr *) &TCPserver, sizeof(struct sockaddr_in)) == -1)
    {
		fprintf(stderr, "ERROR: bind() call failed!\n");
		exit(1);
    }
	// bind a UDP 
    if(bind(UDPsocket, (struct sockaddr *) &UDPserver, sizeof(UDPserver)) == -1)
    {
		fprintf(stderr, "ERROR: UDP bind() call failed!\n");
		exit(1);
    }
    // start listening for incoming connections from clients 
    if(listen(parentsockfd, 5) == -1)
    {
		fprintf(stderr, "ERROR: listen() call failed!\n");
		exit(1);
    }
    // initialize message strings just to be safe (null-terminated) 
    bzero(TCPmessagein, MAX_MESSAGE_LENGTH);
    bzero(TCPmessageout, MAX_MESSAGE_LENGTH);
    fprintf(stderr, "Hello there! I am the vowelizer server!!\n");
    fprintf(stderr, "I am running on TCP port %d and UDP port %d...\n\n", TCP_PORT, UDP_PORT);

    // Main loop: server loops forever listening for requests 
    for( ; ; )
    {
		bzero(UDPmessagein, MAX_MESSAGE_LENGTH);
		bzero(UDPmessageout, MAX_MESSAGE_LENGTH);
		// accept a connection 
		if((childsockfd = accept(parentsockfd, NULL, NULL)) == -1)
		{
			fprintf(stderr, "ERROR: accept() call failed!\n");
			exit(1);
		}
		// try to create a child process to deal with this new client 
		pid = fork();
		// use process id (pid) returned by fork to decide what to do next
		if(pid < 0)
		{
			fprintf(stderr, "ERROR: fork() call failed!!\n");
			exit(1);
		}
		else if(pid == 0)
		{
			// don't need the parent listener socket that was inherited */
			close(parentsockfd);

			// obtain the message from this client */
			while(recv(childsockfd, TCPmessagein, MAX_MESSAGE_LENGTH, 0) > 0)
			{
				if(recvfrom(UDPsocket, UDPmessagein, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *) &client, &len) < 0)
				{
					printf("ERROR: UDP recvfrom() error!\n");
					exit(1);
				}
				printf("Child process received TCP word: %s\n", TCPmessagein);
				printf("Child process received UDP word: %s\n", UDPmessagein);
				if(strstr(UDPmessagein, "D"))
				{
					printf("DEVOWEL\n");
					printf("That word has %ld characters!\n", strlen(TCPmessagein));
					devowel(TCPmessagein);
#ifdef SMPL
					printf("The devowled word: %s\n", TCPmessagein);
					printf("The vowled word: %s\n", UDPmessagein);
					// create the outgoing message (as an ASCII string)
					sprintf(TCPmessageout, "%s\n", TCPmessagein);
					sprintf(UDPmessageout, "%s\n", UDPmessagein);
					printf("The devowled wordOUT: %s\n", TCPmessageout);
					printf("The vowled wordOUT: %s\n", UDPmessageout);
					sendto(UDPsocket, UDPmessageout, strlen(UDPmessageout), 0, (struct sockaddr *) &client, len);
#else
					printf("The devowled word: %s\n", advancedMessage);
					printf("The vowled word: %s\n", advancedMessageUDP);
					// create the outgoing message (as an ASCII string) 
					sprintf(TCPmessageout, "%s\n", advancedMessage);
					sprintf(UDPmessageout, "%s\n", advancedMessageUDP);
					sendto(UDPsocket, UDPmessageout, strlen(UDPmessageout), 0, (struct sockaddr *) &client, len);
#endif
				}
				else
				{
					printf("ENVOWEL\n");
					envowel(TCPmessagein, UDPmessagein);
#ifdef SMPL
					printf("The envowled word: %s\n", TCPmessagein);
					// create the outgoing message (as an ASCII string) 
					sprintf(TCPmessageout, "%s\n", TCPmessagein);
#else
					printf("The envowled word: %s\n", advancedMessage);
					// create the outgoing message (as an ASCII string) 
					sprintf(TCPmessageout, "%s\n", advancedMessage);
#endif
				}
				//printf("Sending word: '%s' of length: %d\n", TCPmessageout,  strlen(TCPmessageout));
				// send the result message back to the client 
				send(childsockfd, TCPmessageout, strlen(TCPmessageout), 0);
				
				// clear out message strings again to be safe */
				bzero(TCPmessagein, MAX_MESSAGE_LENGTH);
				bzero(TCPmessageout, MAX_MESSAGE_LENGTH);
				bzero(UDPmessageout, MAX_MESSAGE_LENGTH);
				bzero(UDPmessagein, MAX_MESSAGE_LENGTH);
			}
			// the socket can be closed and the child process terminated
			close(childsockfd);
			close(UDPsocket);
			exit(0);
		} // end of then part for child 
		else
		{
	    // the parent process is the one doing the "else" part 
	    fprintf(stderr, "Created child process %d to handle that client\n", pid);
	    fprintf(stderr, "Parent going back to job of listening...\n\n");
	    // parent doesn't need the childsockfd
	    close(childsockfd);
	  }
    }
}

void devowel(char* message)
{
#ifdef SMPL
	char* it = message;
	for(int i = 0; i < strlen(message) && it != NULL; i++)
	{
		if((*it == 'a') || (*it == 'A') || (*it == 'e') || (*it == 'E') || (*it == 'i') || (*it == 'I') || (*it == 'o') || (*it == 'O') || (*it == 'u') || (*it == 'U'))
			UDPmessagein[vindex] = *it;
		else
			UDPmessagein[vindex] = ' ';
		vindex++;
		it++;	
	}
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
	//Uppercase
	iter = strchr(message, 'A');
	while(iter != NULL)
	{
		strncpy(iter, " ", 1);
		iter = strchr(message, 'A');
	}
	
	iter = strchr(message, 'E');
	while(iter != NULL)
	{
		strncpy(iter, " ", 1);
		iter = strchr(message, 'E');
	}
	
	iter = strchr(message, 'I');
	while(iter != NULL)
	{
		strncpy(iter, " ", 1);
		iter = strchr(message, 'I');
	}
	
	iter = strchr(message, 'O');
	while(iter != NULL)
	{
		strncpy(iter, " ", 1);
		iter = strchr(message, 'O');
	}
	
	iter = strchr(message, 'U');
	while(iter != NULL)
	{
		strncpy(iter, " ", 1);
		iter = strchr(message, 'u');
	}
#else
	char* it = message;
	char* iter = message;
	int len = 0;
	for(int i = 0; i < strlen(message) && it != NULL; i++)
	{
		if((*it != 'a') && (*it != 'A') && (*it != 'e') && (*it != 'E') && (*it != 'i') && (*it != 'I') && (*it != 'o') && (*it != 'O') && (*it != 'u') && (*it != 'U'))
		{
			advancedMessage[len] = *it;
			len++;
		}
		if((*it == 'a') || (*it == 'A') || (*it == 'e') || (*it == 'E') || (*it == 'i') || (*it == 'I') || (*it == 'o') || (*it == 'O') || (*it == 'u') || (*it == 'U'))
			UDPmessagein[vindex] = *it;
		else
			UDPmessagein[vindex] = ' ';
		vindex++;
		it++;	
	}
	int udplen = 0;
	for(int i = 0; i < strlen(UDPmessagein); i++)
	{
		if(UDPmessagein[i] == ' ')
		{
			if(UDPmessagein[i+1] == ' ')
			{
				if(UDPmessagein[i+2] == ' ')
				{
					if(UDPmessagein[i+3] == ' ')
					{
						advancedMessageUDP[udplen] = '4';
						i = i + 3;
					}
					else
					{
						advancedMessageUDP[udplen] = '3';
						i = i + 2;
					}
				}
				else
				{
					advancedMessageUDP[udplen] = '2';
					i = i + 1;
				}
			}
			else
			{
				advancedMessageUDP[udplen] = '1';
			}
		}
		else
		{
			advancedMessageUDP[udplen] = UDPmessagein[i];
		}
		udplen++;
	}
	advancedMessage[len] = '\0';
	advancedMessageUDP[len] = '\0';
#endif
}

void envowel(char* nonvowels, char* vowels)
{
#ifdef SMPL
	char* it = nonvowels;
	char* iter = vowels;
	int l = 0;
	while(it != NULL && l < strlen(vowels))
	{
		if((*it == ' ') && (*iter != ' '))
		{
			char c = *iter;
			*it = c;
		}
		it++;
		iter++;
		l++;
	}				
#else
	char* it = nonvowels;
	char* iter = strtok(vowels, "      ");
	printf("NonVowels: %s\n", iter);
	printf("Vowels: %s\n", it);
	printf("NonVowels: %s\n", iter);
	for(int i = 0; i < (strlen(nonvowels)+strlen(vowels)); i++)
	{
		printf("ENVOWELED: %s\n", advancedMessage);
		printf("NonVowels: %s\n", iter);
		if(*iter == '1')
		{
			advancedMessage[i] = *it;
			it++;
		}
		else if (*iter == '2')
		{
			for(int j = 0; j < 2; j++)
			{
				advancedMessage[i+j] = *it;
				it++;
			}
			i = i + 1;
		}
		else if(*iter == '3')
		{
			for(int j = 0; j < 3; j++)
			{
				advancedMessage[i+j] = *it;
				it++;
			}
			i = i + 2;
		}
		else if(*iter == '4')
		{
			for(int j = 0; j < 4; j++)
			{
				advancedMessage[i+j] = *it;
				it++;
			}
			i = i + 3;
		}
		else
		{
			advancedMessage[i] = *iter;
		}
		iter++;
	}
	advancedMessage[strlen(advancedMessage)] = *it;
	advancedMessage[strlen(advancedMessage)+1] = '\0';	
#endif
}	