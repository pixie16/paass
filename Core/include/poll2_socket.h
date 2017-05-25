/** \file poll2_socket.h
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

#ifndef POLL2_SOCKET_H
#define POLL2_SOCKET_H

#include <netinet/in.h>
#include <sys/time.h>

#define POLL2_SOCKET_VERSION "1.1.01"
#define POLL2_SOCKET_DATE "May 11th, 2015"

class Server {
private:
    int sock, length, n;
    socklen_t fromlen;
    struct sockaddr_in serv;
    struct sockaddr_in from;
    bool init;

    int to_sec, to_usec;
    fd_set readfds, masterfds;
    struct timeval timeout;

public:
    Server() { init = false; }

    ~Server() { Close(); }

    int Get() { return sock; }

    ///Initialize the serv object and open a specified port. Returns false if the socket fails to open or the socket fails to bind and returns true otherwise.
    bool Init(int port_, int sec_ = 10, int usec_ = 0);

    /** Receive a message from the socket. Returns the number of bytes received. Returns
      * -1 if the receive fails or if the object was not initialized. */
    int RecvMessage(char *message_, size_t length_);

    /** Send a message to the socket. Returns the number of bytes sent. Returns
      * -1 if the send fails or if the object was not initialized. */
    int SendMessage(char *message_, size_t length_);

    bool Select(int &retval);

    /// Close the socket.
    void Close();
};

class Client {
private:
    int sock, n;
    unsigned int length;
    struct sockaddr_in serv, from;
    struct hostent *hp;
    bool init;

public:
    Client() { init = false; }

    ~Client() { Close(); }

    int Get() { return sock; }

    /** Initialize the client object and open a specified address and port. Returns false
      * if the socket fails to open or the address is unresolved and returns true otherwise. */
    bool Init(const char *address_, int port_);

    /** Receive a message from the socket. Returns the number of bytes received. Returns
      * -1 if the receive fails or if the object was not initialized. */
    int RecvMessage(char *message_, size_t length_);

    /** Send a message to the socket. Returns the number of bytes sent. Returns
      * -1 if the send fails or if the object was not initialized. */
    int SendMessage(char *message_, size_t length_);

    /// Close the socket.
    void Close();
};

#endif
