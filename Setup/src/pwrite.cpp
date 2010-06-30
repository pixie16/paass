/********************************************************************/
/*	pwrite.cpp						    */
/*		last updated: 09/30/09 DTM 	     	       	    */
/*			       					    */
/********************************************************************/

#include <cstdio>
#include <cstdlib>

#include "PixieInterface.h"

int main(int argc, char *argv[])
{
  int ModNum, ChanNum;
  double wpar;
  char *NAME;
  bool hadError = false;

  if (argc < 5) {
    printf("usage: %s <module> <channel> <parameter name> <value>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  ModNum = atoi(argv[1]);
  ChanNum = atoi(argv[2]);
  NAME = argv[3];
  wpar = atof(argv[4]);

  PixieInterface pif("pixie.cfg");

  pif.GetSlots();
  pif.Init();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  if (ModNum < 0) {
    // write to all modules, all channels
    for (ModNum = 0; ModNum < pif.GetNumberCards(); ModNum++) {
      for (ChanNum = 0; ChanNum < 16; ChanNum++) {
	if (pif.WriteSglChanPar(NAME, wpar, ModNum, ChanNum)) {
	  pif.PrintSglChanPar(NAME, ModNum, ChanNum);
	} else {
	  hadError = true;
	}
      }
    }
  } else if (ModNum >= 0) {
    if (ChanNum >= 0) {
      if (pif.WriteSglChanPar(NAME, wpar, ModNum, ChanNum)) {
	pif.PrintSglChanPar(NAME, ModNum, ChanNum);
      } else {
	hadError = true;
      }
    } else if (ChanNum < 0) {
      for (ChanNum = 0; ChanNum < 16; ChanNum++) {
	if (pif.WriteSglChanPar(NAME, wpar, ModNum, ChanNum)) {
	  pif.PrintSglChanPar(NAME, ModNum, ChanNum);	
	} else {
	  hadError = true;
	}
      } // channel loop
    } // ChanNum +/-
  } // ModNum +/-  
  
  if (!hadError)
    pif.SaveDSPParameters();

  return EXIT_SUCCESS;
}
