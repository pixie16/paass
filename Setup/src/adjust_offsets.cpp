/** Program which adjusts the baselines of a pixie module to a reasonable value 
 *
 * David Miller, May 2010
 */

#include <cstdio>
#include <cstdlib>

#include "PixieInterface.h"

int main(int argc, char **argv)
{
  int modNum;
  bool hadError = false;

  if (argc != 2) {
    printf("usage: %s <module>\n", argv[0]);
    return EXIT_FAILURE;
  }

  modNum = atoi(argv[1]);

  PixieInterface pif("pixie.cfg");

  pif.GetSlots();
  pif.Init();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  char parName[] = "VOFFSET";

  if (modNum < 0) {
    for (modNum = 0; modNum < pif.GetNumberCards(); modNum++) {
      if (pif.AdjustOffsets(modNum)) {
	for (size_t chanNum = 0; chanNum < pif.GetNumberChannels(); chanNum++) {
	  pif.PrintSglChanPar(parName, modNum, chanNum);
	}
      } else {
	hadError = true;
      }
    }
  } else {
    if (pif.AdjustOffsets(modNum)) {
      for (size_t chanNum = 0; chanNum < pif.GetNumberChannels(); chanNum++) {
	pif.PrintSglChanPar(parName, modNum, chanNum);
      }
    } else {
      hadError = true;
    }
  }

  if (!hadError)
    pif.SaveDSPParameters();

  return EXIT_SUCCESS;
}
