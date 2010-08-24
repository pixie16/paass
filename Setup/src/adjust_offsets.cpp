/** Program which adjusts the baselines of a pixie module to a reasonable value 
 *
 * David Miller, May 2010
 */

#include <cstdio>
#include <cstdlib>

#include "utilities.h"
#include "PixieInterface.h"

class OffsetAdjuster : public PixieFunction<int>
{
  bool operator()(PixieFunctionParms<int> &par);
};

int main(int argc, char **argv)
{
  if (argc != 2) {
    printf("usage: %s <module>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int mod = atoi(argv[1]);

  PixieInterface pif("pixie.cfg");

  pif.GetSlots();
  pif.Init();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  OffsetAdjuster adjuster;

  if (forModule<int>(pif, mod, adjuster)) {
    pif.SaveDSPParameters();
  }

  return EXIT_SUCCESS;
}

bool OffsetAdjuster::operator()(PixieFunctionParms<int> &par)
{
  bool hadError = par.pif.AdjustOffsets(par.mod);
  for (size_t ch = 0; ch < par.pif.GetNumberChannels(); ch++) {
    par.pif.PrintSglChanPar("VOFFSET", par.mod, ch);
  }

  return hadError;
}
