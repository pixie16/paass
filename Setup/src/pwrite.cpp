/********************************************************************/
/*	pwrite.cpp						    */
/*		last updated: 09/30/09 DTM 	     	       	    */
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

class ParameterWriter : public PixieFunction< pair<string, float> >
{
  bool operator()(PixieFunctionParms< pair<string, float> > &par);
};

int main(int argc, char *argv[])
{
  if (argc < 5) {
    cout << "usage: " << argv[0] 
	 << " <module> <channel> <parameter name> <value>" << endl;
    exit(EXIT_FAILURE);
  }
  int mod = atoi(argv[1]);
  int ch  = atoi(argv[2]);
  string parName(argv[3]);
  float val = atof(argv[4]);

  PixieInterface pif("pixie.cfg");

  pif.GetSlots();
  pif.Init();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  ParameterWriter writer;

  if (forChannel(pif, mod, ch, writer, make_pair(parName, val) ) )
    pif.SaveDSPParameters();

  return EXIT_SUCCESS;
}

bool ParameterWriter::operator()(PixieFunctionParms< pair<string, float> > &par)
{
  if (par.pif.WriteSglChanPar(par.par.first.c_str(), par.par.second,
			      par.mod, par.ch) ) {
    par.pif.PrintSglChanPar(par.par.first.c_str(), par.mod, par.ch);
    return true;
  }
  return false;
}
