/********************************************************************/
/*	pread.cpp      					            */
/*		last updated: 09/30/09 DTM	     	       	    */
/*			       					    */
/********************************************************************/
#include <string>

#include <cstdio>
#include <cstdlib>

#include "utilities.h"
#include "PixieInterface.h"

using std::string;

class ParameterReader : public PixieFunction<string>
{
public:
  bool operator()(PixieFunctionParms<string> &par);
};

int main(int argc, char *argv[])
{
  if (argc != 4) {
    printf("usage: %s <module> <channel> <parameter name>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  int mod = atoi(argv[1]);
  int ch  = atoi(argv[2]);
  string parName(argv[3]);

  PixieInterface pif("pixie.cfg");

  pif.GetSlots();
  pif.Init();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  ParameterReader reader;

  forChannel<string>(pif, mod, ch, reader, parName);

  return EXIT_SUCCESS;
}

bool ParameterReader::operator()(PixieFunctionParms<string> &par)
{
  par.pif.PrintSglChanPar(par.par.c_str(), par.mod, par.ch);

  return true;
}
