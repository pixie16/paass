/** \file poll2_socket.cpp
  * 
  * \brief Provides network connectivity for poll2
  * 
  * \author Cory R. Thornsberry
  * 
  * \date April 20th, 2015
  * 
  * This file contains classes used by poll2 in order to send packets over the
  * network. These packets are sent by poll using the Server class and may be
  * read by any program using the Client class or operating a socket on the
  * same port.
*/

#include "poll2_socket.h"

#include <iostream>
#include <string>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

/////////////////////////////////////////////////////////////////////
// class Server
/////////////////////////////////////////////////////////////////////
/**
 *	\param[in] port_ The port number to use for the connection.
 *	\param[in] sec_ The number of seconds to wait for recieve timeout message.
 *	\param[in] usec_ The number of microseconds in addition to the above seconds to wait for recieve timeout message.
 *	\return Returns false if the socket fails to open or bind.
 */
bool Server::Init(int port_, int sec_, int usec_) {
    if (init) { return false; }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { return false; } // failed to open socket

    length = sizeof(serv);
    bzero(&serv, length);

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(port_);

    if (bind(sock, (struct sockaddr *) &serv, length) <
        0) { return false; } // failed to bind to port

    fromlen = sizeof(struct sockaddr_in);

    to_sec = sec_;
    to_usec = usec_;

    return init = true;
}

int Server::RecvMessage(char *message_, size_t length_) {
    if (!init) { return -1; }

    // Zero the message
    bzero(message_, length_);

    int nbytes = (int) recvfrom(sock, message_, length_, 0,
                                (struct sockaddr *) &from, &fromlen);
    return nbytes;
}

int Server::SendMessage(char *message_, size_t length_) {
    if (!init) { return -1; }

    return (int) sendto(sock, message_, length_, 0, (struct sockaddr *) &from,
                        fromlen);
}

bool Server::Select(int &retval) {
    timeout.tv_sec = to_sec; // Set timeout to sec_
    timeout.tv_usec = to_usec;

    FD_ZERO(&masterfds);
    FD_SET(sock, &masterfds);
    memcpy(&readfds, &masterfds, sizeof(fd_set));

    retval = select(sock + 1, &readfds, NULL, NULL, &timeout);
    if (FD_ISSET(sock, &readfds)) { return true; }

    return false;
}

void Server::Close() {
    if (!init) { return; }

    close(sock);
}

/////////////////////////////////////////////////////////////////////
// class Client
/////////////////////////////////////////////////////////////////////

bool Client::Init(const char *address_, int port_) {
    if (init) { return false; }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { return false; } // failed to open socket

    serv.sin_family = AF_INET;
    hp = gethostbyname(address_);
    if (!hp) { return false; } // failed to resolve hostname

    bcopy((char *) hp->h_addr, (char *) &serv.sin_addr, hp->h_length);
    serv.sin_port = htons(port_);
    length = sizeof(struct sockaddr_in);

    return init = true;
}

int Client::RecvMessage(char *message_, size_t length_) {
    if (!init) { return -1; }

    // Zero the message
    bzero(message_, length_);

    return (int) recvfrom(sock, message_, length_, 0, (struct sockaddr *) &from,
                          &length);
}

int Client::SendMessage(char *message_, size_t length_) {
    if (!init) { return -1; }

    return (int) sendto(sock, message_, length_, 0,
                        (const struct sockaddr *) &serv, length);
}

void Client::Close() {
    if (!init) { return; }

    close(sock);
}
