#ifndef __MAIN_MONITOR_HPP
#define __MAIN_MONITOR_HPP
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstdlib>

#include "monitor.hpp"
#include "poll2_socket.h"

std::vector<Client>* Submonitor_Client_List;  // must be global for signal_handlers
bool DumMode=false;

using namespace std;
class mainmonitor : public monitor {
   public:
    mainmonitor();
    ~mainmonitor();
    void OpenSubmonitorSockets();
    vector<Client>* GetSubClientVec() { return &Submonitor_Clients; };  // return the vec of submonitor poll clients
    Server* GetPollServer() { return poll_server; };                    // return the poll server

    void RelayPoll2msg(char* buffer, size_t msg_size);

    void CloseSubmonitors();



   private:
    Server* poll_server;
    string name;
    vector<Client> Submonitor_Clients;

    


};

#endif