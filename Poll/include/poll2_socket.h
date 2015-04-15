#ifndef POLL2_SOCKET_H
#define POLL2_SOCKET_H

#include <string>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Initialize a client for poll2 running on remote a machine at address_:port_
bool init_client(const char* address_, int port_);

// Receive a message from the server
int client_recv_message(char *message_ , size_t length_);

int client_recv_message(std::string &message, size_t max_length_);

// Close the client socket
bool close_client();

// Initialize a server on a specified port
bool init_server(int portNum_);

// Send a message to the socket
int server_send_message(char *message_, size_t length_);

// Receive a message from the socket
int server_recv_message(char *message_, size_t length_);

// Close the server socket
bool close_server();

#endif
