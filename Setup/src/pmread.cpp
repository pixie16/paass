/********************************************************************/
/*	pmread.cpp	       				            */
/*		last updated: 09/30/09 DTM	     	       	    */
/*			       					    */
/********************************************************************/
#include <cstdio>
#include <cstdlib>

#include "PixieInterface.h"

int main(int argc, char *argv[])
{
  unsigned long ModNum;
  char *NAME;

  if (argc != 3) {
    printf("usage: %s <module> <parameter name>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  ModNum = atol(argv[1]);
  NAME = argv[2];

  PixieInterface pif("pixie.cfg");

  pif.GetSlots();
  pif.Init();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  if (ModNum < 0) {
    // print the parameter for all modules
    for (ModNum = 0; ModNum < pif.GetNumberCards(); ModNum++) {
      pif.PrintSglModPar(NAME, ModNum);
    }
  } else if (ModNum >= 0) {
    pif.PrintSglModPar(NAME, ModNum);
  }

  return EXIT_SUCCESS;
}
