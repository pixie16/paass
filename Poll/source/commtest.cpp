// PACOR communication test
// last updated: February 2010, DTM

#include <iostream>

#include <netinet/in.h>

#define IO_BUFFER_LENGTH 65536 // necessary for Buffer_structure

#include "Acq_Params.h"
#include "Buffer_Structure.h"
#include "acq_ctl.h"
#include "orph_udp.h"
#include "spkt_io_udp.h"

const char *VME = "127.0.0.1";

using namespace std;

int main(int argc, char **argv)
{
  spkt_connect(VME, PROTO_DATA);
  cout << "Connected to PAC-machine " << VME << endl;

  // open a socket for command communication
  if (spkt_open("", PROTO_FECNTRL) == -1) {
    // failed to open socket
    cout << "Failed to open socket" << endl;
  } else {
    cout << "Socket opened" << endl;
  }

  UDP_Packet command;

  bool isExiting = false;
  // wait for start command
  while (!isExiting) {
    command.DataSize = 100;
    if (spkt_recv(&command) == -1) {
      // failed to receive packet
    } else {
      cout << "Received command " << (int)(command.Data[0]) << endl;
    }
    switch (command.Data[0]) {
    case INIT_ACQ:
      command.Data[0] = ACQ_OK;
      spkt_send(&command);
      isExiting = true;
      break;
    case START_ACQ:
      socket_poll(0);
      command.Data[0] = ACQ_OK;
      spkt_send(&command); 
      break;
    case ZERO_CLK:
      command.Data[0] = ACQ_OK;
      spkt_send(&command);
      return 0;
    case STOP_ACQ:
      command.Data[0] = ACQ_STP_HALT;
      spkt_send(&command);
      break;
    case STATUS_ACQ:
      command.Data[0] = ACQ_OK;
      command.Data[1] = ACQ_STOP;
      spkt_send(&command);
      break;
    default:
      command.Data[0] = ACQ_UNKNOWN_COMMAND;
      spkt_send(&command);
      break;
    } // end switch
  } // end while waiting for start signal

  spkt_close();
}
