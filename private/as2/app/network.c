#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
#define MSG_MAX_LEN 1024
#define PORT        2000

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
    printf("Brian's Net Listen Test on UDP port %d:\n", PORT);
	printf("Connect using: \n");
	printf("    netcat -u 127.0.0.1 %d\n", PORT);

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
        perror("Failed to create thread in network_listener");
        return 1;
    }
    return 0;
}

void network_clean(void)
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

		// Make it null terminated (so string functions work)
		// - recvfrom given max size - 1, so there is always room for the null
		messageRx[bytesRx] = 0;
		printf("Message received (%d bytes): \n\n'%s'\n", bytesRx, messageRx);
		
		// Extract the value from the message:
		// (process the message any way your app requires).
		int incMe = atoi(messageRx);

		// Compose the reply message:
		// (NOTE: watch for buffer overflows!).
		char messageTx[MSG_MAX_LEN];
		sprintf(messageTx, "Math: %d + 1 = %d\n", incMe, incMe + 1);

		// Transmit a reply:
		sin_len = sizeof(sinRemote);
		sendto( socketDescriptor,
			messageTx, strlen(messageTx),
			0,
			(struct sockaddr *) &sinRemote, sin_len);

		if (strncmp(messageRx, "quit", strlen("quit")) == 0) {
			printf("That's it! Lecture is done!\n");
			break;
		}
	}
    return 0;
}