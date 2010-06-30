/********************************************************************/
/*	trace.cpp 						    */
/*		last updated: 10/02/09 DTM	     	       	    */
/*			       					    */
/********************************************************************/
#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <cmath>

// pixie includes
#include "PixieInterface.h"

using std::cout;
using std::endl;

int main(int argc, char **argv)
{
  unsigned int ModNum = -1, ChanNum = -1;
  const size_t size = PixieInterface::GetTraceLength();

  unsigned short Trace[size];
  unsigned long trace_aver;
  unsigned long trace_sig;

  PixieInterface pif("pixie.cfg");

  pif.GetSlots();
  pif.Init();

  if (argc > 1) {
    ModNum = atoi(argv[1]);
    if (argc > 2) {
      ChanNum = atoi(argv[2]);
    }
  }
  
  usleep(200);
  /*
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);
  */
  pif.Boot(0x7f,true);

  if (ModNum < pif.GetNumberCards()) {
    pif.AcquireTraces(ModNum);
    for (ChanNum = 4; ChanNum < 5; ChanNum++) {
    //    for (ChanNum = 0; ChanNum < pif.GetNumberChannels(); ChanNum++) {
      usleep(10);

      if (!pif.ReadSglChanTrace(Trace, size, ModNum, ChanNum))
	continue;

      trace_aver = 0;
      trace_sig = 0;
      for (size_t chan = 0; chan < size; chan++) {
	trace_aver += Trace[chan];
	if (ChanNum == 4)
	  cout << chan << " " << Trace[chan] << endl;

      }
      trace_aver /= size;

      for (size_t chan = 0; chan < size; chan++) {
	trace_sig += (Trace[chan] - trace_aver) * (Trace[chan] - trace_aver);
      }
      trace_sig = sqrt(trace_sig / size);

      printf("Trace ---- MOD/CHAN %2u / %2u AVER |-  %4lu  -| SIG %2lu \n",ModNum,ChanNum,trace_aver,trace_sig);
    }
  } else {
    for(ModNum = 0; ModNum < pif.GetNumberCards(); ModNum ++) {
      pif.AcquireTraces(ModNum);
      for(ChanNum = 0; ChanNum < pif.GetNumberChannels(); ChanNum ++) {
	usleep(10);
	if (!pif.ReadSglChanTrace(Trace, size ,ModNum, ChanNum))
	  continue;
	
	trace_aver=0;
	trace_sig=0;
	for(size_t chan = 0; chan < size; chan ++) {
	  trace_aver += Trace[chan];
	}
	trace_aver=trace_aver/size;
	
	
	for(size_t chan = 0; chan < size; chan ++) {
	  trace_sig += (trace_aver-Trace[chan])*(trace_aver-Trace[chan]);
	}
	trace_sig =(long)sqrt(trace_sig / size);

	printf("Trace ---- MOD/CHAN %2u / %2u AVER |-  %4lu  -| SIG %2lu \n",ModNum,ChanNum,trace_aver,trace_sig);
      }
    }
  }

  return EXIT_SUCCESS;
}
