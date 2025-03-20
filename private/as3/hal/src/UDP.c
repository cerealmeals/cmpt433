#include "UDP.h"
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
#include <signal.h>
#define MSG_MAX_LEN 1024
#define PORT        12345

static void * thread_function(void* arg); 

// Lots of this code was taken from https://opencoursehub.cs.sfu.ca/bfraser/solutions/433/06-linuxprogramming/demo_udpListen.c
// but edited by me Sam

// Address
static struct sockaddr_in sin;
static int socketDescriptor;
static pthread_t netword_thread;
static bool is_init = false;
static bool running = false;

static network_callback_t network_callback = NULL;

void UDP_init(network_callback_t callback)
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
    network_callback = callback;
    running = true;
	if (pthread_create(&netword_thread, NULL, thread_function, NULL) != 0){
        perror("Failed to create thread in network_listener\n");
        exit(EXIT_FAILURE);
    }
    
    is_init = true;
}


void UDP_cleanup(void)
{
    assert(is_init == true);

    pthread_kill(netword_thread, SIGUSR2);

	// Interrupt recvfrom
    shutdown(socketDescriptor, SHUT_RD);
	
    // make sure the thread ended
    pthread_join(netword_thread, NULL);
	
    // Close
	close(socketDescriptor);
    is_init = false;
}

static void UDP_signal_handler(int sig) {
    if (sig == SIGUSR2) {
        printf("Received SIGUSR2 in UPD, shutting down...\n");
        running = false;
    }
}

static void * thread_function(void* arg)
{
    if (arg != NULL){
        return NULL;
    }
	signal(SIGUSR2, UDP_signal_handler);
    while (running) {
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
		
        char messageTx[MSG_MAX_LEN];
		# if 0
		printf("Message received (%d bytes): \n\n'%s'\n", bytesRx, messageRx);
		#endif

        network_callback(messageRx, messageTx, MSG_MAX_LEN);

        if (strlen(messageTx) > 0) {
            sendto(socketDescriptor, messageTx, strlen(messageTx), 0,
                   (struct sockaddr *) &sinRemote, sin_len);
        }
		
	}
    return 0;
}