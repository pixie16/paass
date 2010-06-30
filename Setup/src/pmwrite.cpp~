/********************************************************************/
/*	pmwrite.cpp						    */
/*		last updated: 12/17/2009 DTM 	     	       	    */
/*			       					    */
/********************************************************************/

#include <iostream>

#include <cstdlib>

#include "PixieInterface.h"

using namespace std;

int main(int argc, char *argv[])
{
  unsigned int modNum;
  unsigned long wpar;
  char *name;
  bool hadError = false;

  if (argc != 4) {
    cout << "usage: " << argv[0] << "<module> <parameter name> <value>" << endl;
    return EXIT_FAILURE;
  }
  PixieInterface pif("pixie.cfg");

  modNum = atoi(argv[1]);
  name = argv[2];
  wpar = atol(argv[3]);

  pif.GetSlots();
  pif.Init();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  if (modNum < 0) {
    // write to all modules
    for (modNum = 0; modNum < pif.GetNumberCards(); modNum++) {
      if (pif.WriteSglModPar(name, wpar, modNum))
	pif.PrintSglModPar(name, modNum);
      else
	hadError = true;
    }
  } else if ( modNum < pif.GetNumberCards() ) {
    if (pif.WriteSglModPar(name, wpar, modNum)) 
      pif.PrintSglModPar(name, modNum);
    else 
      hadError = true;
  } else {
    cout << "Module number out of range." << endl;
    hadError = true;
  }

  if (!hadError) 
    pif.SaveDSPParameters();

  return EXIT_SUCCESS;
}
