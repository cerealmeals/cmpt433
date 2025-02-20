#include "network.h"
#include "shutdown.h"
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
#define MSG_MAX_LEN 1024
#define PORT        12345

static void * thread_function(void* arg); 

// Lots of this code was taken from https://opencoursehub.cs.sfu.ca/bfraser/solutions/433/06-linuxprogramming/demo_udpListen.c
// but edited by me Sam

// Address
static struct sockaddr_in sin;
static int socketDescriptor;
static pthread_t thread;
static bool is_init = false;

void network_init(void)
{
	assert(!is_init);
	printf("Connect using: \n");
	printf("netcat -u 192.168.6.2 %d\n", PORT);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;                   // Connection may be from network
	sin.sin_addr.s_addr = htonl(INADDR_ANY);    // Host to Network long
	sin.sin_port = htons(PORT);                 // Host to Network short
	
	// Create the socket for UDP
	socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

	// Bind the socket to the port (PORT) that we specify
	int e = bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));	

	// Check for errors (-1)
    if (e == -1){
        perror("Error binding the socket");
    }

    is_init = true;
}
int network_listener(void)
{
    assert(is_init == true);
    if (pthread_create(&thread, NULL, thread_function, NULL) != 0){
        perror("Failed to create thread in network_listener\n");
        return 1;
    }
    return 0;
}

void network_cleanup(void)
{
    assert(is_init == true);
    // make sure the thread ended
    pthread_join(thread, NULL);
	
    // Close
	close(socketDescriptor);
    is_init = false;
}

static void * thread_function(void* arg)
{
    if (arg != NULL){
        return NULL;
    }
	char previousMessage[MSG_MAX_LEN] = {"\0"};
    while (1) {
		// Get the data (blocking)
		// Will change sin (the address) to be the address of the client.
		// Note: sin passes information in and out of call!
		struct sockaddr_in sinRemote;
		unsigned int sin_len = sizeof(sinRemote);
		char messageRx[MSG_MAX_LEN];

		// Pass buffer size - 1 for max # bytes so room for the null (string data)
		int bytesRx = recvfrom(socketDescriptor,
			messageRx, MSG_MAX_LEN - 1, 0,
			(struct sockaddr *) &sinRemote, &sin_len);

		// Check for errors (-1)
		if(bytesRx == -1){
			perror("didn't receive any bytes\n");
		}

		// Make it null terminated (so string functions work)
		// - recvfrom given max size - 1, so there is always room for the null
		messageRx[bytesRx] = 0;
		
		# if 0
		printf("Message received (%d bytes): \n\n'%s'\n", bytesRx, messageRx);
		#endif
		
		
		// Extract the value from the message:
		// (process the message any way your app requires).

		// Compose the reply message:
		// (NOTE: watch for buffer overflows!).
		char messageTx[MSG_MAX_LEN];
		
		
		// save the command
		if (strncmp(messageRx, "\n", strlen("\n")) == 0 || strncmp(messageRx, "\r\n", strlen("\r\n")) == 0){
			snprintf(messageRx, sizeof(char)* MSG_MAX_LEN, previousMessage);
		}
		else{
			snprintf(previousMessage, sizeof(char)* MSG_MAX_LEN, messageRx);
		}
		
		if (strncmp(messageRx, "help", strlen("help")) == 0) {
			
			sprintf(messageTx, "Accepted command examples:\n" 
				"%-10s %s\n"
				"%-10s %s\n"
				"%-10s %s\n"
				"%-10s %s\n"
				"%-10s %s\n"
				"%-10s %s\n",
				"count", "-- get the total number of samples taken.",
				"length", "-- get the number of samples taken in the previously completed second.",
				"dips", "-- get the number of dips in the previously completed second.",
				"history", "-- get all the samples in the previously completed second.",
				"stop", "-- cause the server program to end.",
				"<enter>", "-- repeat last command."
			);
			// Transmit a reply:
			sin_len = sizeof(sinRemote);
			sendto( socketDescriptor,
			messageTx, strlen(messageTx),
			0,
			(struct sockaddr *) &sinRemote, sin_len);
		}
		else if (strncmp(messageRx, "?", strlen("?")) == 0) {
			
			sprintf(messageTx, "Accepted command examples:\n"
				"%-10s %s\n" 
				"%-10s %s\n"
				"%-10s %s\n"
				"%-10s %s\n"
				"%-10s %s\n"
				"%-10s %s\n",
				"count", "-- get the total number of samples taken.",
				"length", "-- get the number of samples taken in the previously completed second.",
				"dips", "-- get the number of dips in the previously completed second.",
				"history", "-- get all the samples in the previously completed second.",
				"stop", "-- cause the server program to end.",
				"<enter>", "-- repeat last command."
			);
			// Transmit a reply:
			sin_len = sizeof(sinRemote);
			sendto( socketDescriptor,
			messageTx, strlen(messageTx),
			0,
			(struct sockaddr *) &sinRemote, sin_len);
		}
		else if (strncmp(messageRx, "count", strlen("count")) == 0) {
			
			sprintf(messageTx, "count\n");
			// Transmit a reply:
			sin_len = sizeof(sinRemote);
			sendto( socketDescriptor,
			messageTx, strlen(messageTx),
			0,
			(struct sockaddr *) &sinRemote, sin_len);
		}
		else if (strncmp(messageRx, "length", strlen("length")) == 0) {
			
			sprintf(messageTx, "length\n");
			// Transmit a reply:
			sin_len = sizeof(sinRemote);
			sendto( socketDescriptor,
			messageTx, strlen(messageTx),
			0,
			(struct sockaddr *) &sinRemote, sin_len);
		}
		else if (strncmp(messageRx, "dips", strlen("dips")) == 0) {
			
			sprintf(messageTx, "dips\n");
			// Transmit a reply:
			sin_len = sizeof(sinRemote);
			sendto( socketDescriptor,
			messageTx, strlen(messageTx),
			0,
			(struct sockaddr *) &sinRemote, sin_len);
		}
		else if (strncmp(messageRx, "history", strlen("history")) == 0) {
			
			sprintf(messageTx, "history\n");
			// Transmit a reply:
			sin_len = sizeof(sinRemote);
			sendto( socketDescriptor,
			messageTx, strlen(messageTx),
			0,
			(struct sockaddr *) &sinRemote, sin_len);
		}
		else if (strncmp(messageRx, "stop", strlen("stop")) == 0) {
			printf("Program terminating.\n");

			sprintf(messageTx, "Program terminating.\n");
			// Transmit a reply:
			sin_len = sizeof(sinRemote);
			sendto( socketDescriptor,
			messageTx, strlen(messageTx),
			0,
			(struct sockaddr *) &sinRemote, sin_len);

			shutdown_shutdown();
			break;
		}
		else{
			sprintf(messageTx, "Unkown Command\n");
			// Transmit a reply:
			sin_len = sizeof(sinRemote);
			sendto( socketDescriptor,
			messageTx, strlen(messageTx),
			0,
			(struct sockaddr *) &sinRemote, sin_len);
		}
	}
    return 0;
}