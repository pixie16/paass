///@file ExtTSSenderProcessor.hpp
///@brief A class to send external timestamp info of a given detector
/// to a server by udp
///@author R. Yokoyama
///@date 23 May 2018

#ifndef __EXTTSSENDERPROCESSOR_HPP_
#define __EXTTSSENDERPROCESSOR_HPP_

#include <iostream>
#include <string>
#include <vector>
#include "poll2_socket.h"
#include "EventProcessor.hpp"
#include "RawEvent.hpp"

class ExtTSSenderProcessor : public EventProcessor {

private:
    std::string type_; //!< A type of the detector to send timestamp
    std::string tag_; //!< A tag of the ch to send timestamp
    int port_; //!< A port number to communicate
    std::string hostName_; //!< The host name of the server
    Client* udpClient_; //!< A udp client in poll2_socket.h
    unsigned long long int *buffer_; //!< A data buffer for sending
    unsigned int buffSize_; //!< A buffer size in 64 bit
    unsigned int curPos_; //!< A counter

public:
    // Default constructor
    ExtTSSenderProcessor();

    // Constructor taking the detector type and udp settings as arguments
    // @param [in] type : the type of the detector to send its timestamps
    // @param [in] hostName : the host name of the server to send the timestamps
    // @param [in] port : the port number of the server to send the timestamps
    // @param [in] buffSize : the size of the buffer in 64 bit integer
    ExtTSSenderProcessor(const std::string &type, const std::string &hostName,
                         const std::string &tag, const int &port, const int &buffSize);

    // Default destructor
    ~ExtTSSenderProcessor();
 
    // Process the data and send external timestamp of given detector
    virtual bool Process(RawEvent &event);

    //Set buffSize_ and recreate the buffer_
    void SetBuffSize(const unsigned int &buffSize);

    //Initialization
    bool Init(const std::string &host_name, const int &port);

    //To be called each time to fill a timestamp to be sent later when the buffer
    //becomes full
    void SetTS(const unsigned long long int &ts);

    //Send a whole buffer content with udp to the server
    int SendTS();

    //Close udp client
    void Close(){udpClient_->Close();}

    //Clear all content of the buffer to 0
    void ClearBuff();

    int GetPort() const { return port_; }
    std::string GetHostName() const { return hostName_; }
};

#endif //__EXTTSSENDERPROCESSOR_HPP_
