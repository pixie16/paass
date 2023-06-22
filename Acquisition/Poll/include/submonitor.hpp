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
class submonitor : public monitor {
   public:
    submonitor();
    ~submonitor();

    Server* GetPollServer() { return poll_server; };  // return the poll server

   private:
    Server* poll_server;
    string name;
    vector<Client> Submonitor_Clients;

    int numRowsOfMods = 1;

    


};

#endif