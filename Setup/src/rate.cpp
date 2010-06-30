/********************************************************************/
/*	rate.cpp   						    */
/*		last updated: 10/02/09 DTM	     	       	    */
/*			       					    */
/********************************************************************/

#include <cstdio>
#include <cstdlib>

#include "PixieInterface.h"

int main(int argc, char *argv[])
{
  int modNum, chanNum;
  double inputRate, outputRate, liveTime, procEvents, realTime;

  if (argc != 3) {
    printf("usage: %s <module> <channel>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  modNum  = atoi(argv[1]);
  chanNum = atoi(argv[2]);

  PixieInterface pif("pixie.cfg");
  pif.GetSlots();
  pif.Init();
  pif.Boot(0, true);

  printf(" %2s %2s  %10s  %10s  %10s  %10s  %10s\n",
	 "M", "C", "Input",  "Output", "Live_t", "Proc_ev", "RealTime");

  if (modNum < 0) {
    for (modNum = 0; modNum < pif.GetNumberCards(); modNum++) {
      if (pif.GetStatistics(modNum)) {
	for (unsigned int chanNum = 0; chanNum < pif.GetNumberChannels(); chanNum++) {
	  inputRate  = pif.GetInputCountRate(modNum, chanNum);
	  outputRate = pif.GetOutputCountRate(modNum, chanNum);
	  liveTime   = pif.GetLiveTime(modNum, chanNum);
	  realTime   = pif.GetRealTime(modNum);
	  procEvents = pif.GetProcessedEvents(modNum);

	  printf(" %2u %2u  %10.1f  %10.1f  %10.1f  %10.0f  %10.1f \n", modNum,
		 chanNum, inputRate, outputRate, liveTime, procEvents,
		 realTime);
	} // for each channel
      }	
    } // for each module
  } else if (modNum >= 0 && pif.GetStatistics(modNum)) {    
    if (chanNum >= 0) {
      inputRate  = pif.GetInputCountRate(modNum, chanNum);
      outputRate = pif.GetOutputCountRate(modNum, chanNum);
      liveTime   = pif.GetLiveTime(modNum, chanNum);
      realTime   = pif.GetRealTime(modNum);
      procEvents = pif.GetProcessedEvents(modNum);
      
      printf(" %2u %2u  %10.1f  %10.1f  %10.1f  %10.0f  %10.1f \n", modNum,
	     chanNum, inputRate, outputRate, liveTime, procEvents,
	     realTime);
    } else if (chanNum < 0) {
      for (unsigned int chanNum = 0; chanNum < pif.GetNumberChannels(); chanNum++) {
	  inputRate  = pif.GetInputCountRate(modNum, chanNum);
	  outputRate = pif.GetOutputCountRate(modNum, chanNum);
	  liveTime   = pif.GetLiveTime(modNum, chanNum);
	  realTime   = pif.GetRealTime(modNum);
	  procEvents = pif.GetProcessedEvents(modNum);

	  printf(" %2u %2u  %10.1f  %10.1f  %10.1f  %10.0f  %10.1f \n", modNum,
		 chanNum, inputRate, outputRate, liveTime, procEvents,
		 realTime);
      } // for each channel     
    } // if arg2 < 0
  } // if arg1 >= 0

  return EXIT_SUCCESS;
}
