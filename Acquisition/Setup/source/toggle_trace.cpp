// toggle_trace.cpp
// Switch whether traces are recorded for channels
// August 2010, DTM

#include <iostream>

#include <cstdlib>

#include "utilities.h"
#include "PixieInterface.h"

using std::cout;
using std::endl;

class TraceToggler : public PixieFunction<>
{
  bool operator()(PixieFunctionParms<> &par);
};

int main(int argc, char **argv)
{
  if (argc != 3) {
    cout << "usage: " << argv[0] << " <module> <channel>" << endl;
    return EXIT_FAILURE;
  }

  int mod = atoi(argv[1]);
  int ch  = atoi(argv[2]);

  PixieInterface pif("pixie.cfg");
  
  pif.GetSlots();
  pif.Init();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);
  
  TraceToggler toggler;
  
  if (forChannel(pif, mod, ch, toggler))
    pif.SaveDSPParameters();

  return EXIT_SUCCESS;
}

bool TraceToggler::operator()(PixieFunctionParms<> &par)
{
#ifdef PIF_REVD
  if ( par.pif.ToggleTraceCapture(par.mod, par.ch) ) {
    par.pif.PrintSglChanPar("CHANNEL_CSRA", par.mod, par.ch);
    return true;
  } else {
    return false;
  }
#else
  cout << "Toggling trace capture only implemented for Rev. D" << endl;
  return false;
#endif
}
