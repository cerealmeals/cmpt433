// This module provides a UDP-based network server that listens for incoming messages, 
// processes commands, and sends appropriate responses. 
// It is designed to interact with a Sampler module to retrieve data and a shutdown module to handle program termination.


#ifndef UDP_H
#define UDP_H

#define MSG_MAX_LEN 1024

// Callback function type: Takes received message, fills response message
typedef void (*network_callback_t)(char* received, char* response, int max_response_len);

// Initializes the UDP server, binds it to a specified port, and starts a background thread to handle incoming messages.
void UDP_init(network_callback_t callback);

// Cleans up resources, joins the background thread, and closes the socket.
void UDP_cleanup(void);

#endif