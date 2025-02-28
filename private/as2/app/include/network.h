// This module provides a UDP-based network server that listens for incoming messages, 
// processes commands, and sends appropriate responses. 
// It is designed to interact with a Sampler module to retrieve data and a shutdown module to handle program termination.


#ifndef NETWORK_H
#define NETWORK_H

// Initializes the UDP server, binds it to a specified port, and starts a background thread to handle incoming messages.
void network_init(void);

// Cleans up resources, joins the background thread, and closes the socket.
void network_cleanup(void);

#endif