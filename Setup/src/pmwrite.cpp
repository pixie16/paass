/********************************************************************/
/*	pmwrite.cpp						    */
/*		last updated: 12/17/2009 DTM 	     	       	    */
/*			       					    */
/********************************************************************/

#include <iostream>
#include <map>
#include <string>

#include <cstdlib>

#include "utilities.h"
#include "PixieInterface.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;

class ParameterWriter : public PixieFunction< pair<string, unsigned long> >
{
  bool operator()(PixieFunctionParms< pair<string, unsigned long> > &par);
};

int main(int argc, char *argv[])
{
  if (argc != 4) {
    cout << "usage: " << argv[0] << "<module> <parameter name> <value>" << endl;
    return EXIT_FAILURE;
  }
  PixieInterface pif("pixie.cfg");

  int    mod = atoi(argv[1]);
  string parName(argv[2]);
  unsigned long val = atol(argv[3]);

  pif.GetSlots();
  pif.Init();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  ParameterWriter writer;
  if ( forModule(pif, mod, writer, make_pair(parName, val) ) )
      pif.SaveDSPParameters();

  return EXIT_SUCCESS;
}

bool ParameterWriter::operator()(PixieFunctionParms< pair<string, unsigned long> > &par)
{
  if (par.pif.WriteSglModPar(par.par.first.c_str(), par.par.second, par.mod)) {
    par.pif.PrintSglModPar(par.par.first.c_str(), par.mod);
    return true;
  } else {
    return false;
  }
}
