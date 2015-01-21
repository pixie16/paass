/********************************************************************/
/*	pmread.cpp	       				            */
/*		last updated: 08/23/10 DTM	     	       	    */
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
  bool operator()(PixieFunctionParms<string> &par);
};

int main(int argc, char *argv[])
{
  if (argc != 3) {
    printf("usage: %s <module> <parameter name>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int    mod = atoi(argv[1]);
  string parName(argv[2]);

  PixieInterface pif("pixie.cfg");

  pif.GetSlots();
  pif.Init();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  ParameterReader reader;
  forModule(pif, mod, reader, parName);

  return EXIT_SUCCESS;
}

bool ParameterReader::operator()(PixieFunctionParms<string> &par)
{
  par.pif.PrintSglModPar(par.par.c_str(), par.mod);
  return true;
}
