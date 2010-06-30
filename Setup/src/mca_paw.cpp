/********************************************************************/
/*	mca_paw.cpp						    */
/*		last updated: 10/02/09 DTM	     	       	    */
/*			       					    */
/********************************************************************/
#include <cstdio>
#include <cstdlib>

// pixie includes
#include "PixieInterface.h"
#include "Utility.h"

// cernlib includes
#include "cfortran.h"
#include "hbook.h"

const size_t PAWC_SIZE = 1200000;
const size_t HIS_SIZE = 32768;

#ifdef __cplusplus
extern "C" {
#endif
  typedef struct {
    float PAW[PAWC_SIZE];
  } PAWC_DEF;
#define PAWC COMMON_BLOCK(PAWC,pawc)
  COMMON_BLOCK_DEF(PAWC_DEF, PAWC);
  
  PAWC_DEF pawc_;
#ifdef __cplusplus
}
#endif

int main(int argc, char *argv[])
{
  PixieInterface::word_t histo[HIS_SIZE];

  int sec, time;
  double timer, start_mca;

  if (argc >= 2) {
    time = atoi(argv[1]);
  } else {
    time = 10;
  }

  PixieInterface pif("pixie.cfg");
  pif.GetSlots();

  HLIMIT(PAWC_SIZE);

  pif.Init();

  //cxx, end any ongoing runs
  pif.EndRun();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  // pif.WriteSglModPar("HOST_RT_PRESET", Decimal2IEEEFloating(cmd1), 0);
  pif.RemovePresetRunLength(0);

  pif.StartHistogramRun();
  start_mca = usGetTime(0);
  printf("|Start MCA %32.1f us |\n", start_mca);

  usleep(100);
  for (sec = 0; sec < time; sec++) {
    sleep(1);
    printf("|%u s |\n", sec);
    if (!pif.CheckRunStatus()) {
      printf("Run \e[47;31mTERMINATED\e[0m\n");
      break;
    }
  }
  printf("\n");

  pif.EndRun();

  timer = usGetTime(start_mca);
  printf("|Stop MCA %32.3f s |\n", timer / 1000000.);

  usleep(100);

  char hisName[] = "test spectrum";
  char hisFile[] = "mca.dat";

  for (size_t ModNum = 0; ModNum < pif.GetNumberCards(); ModNum++) {
    for (size_t ChanNum = 0; ChanNum < pif.GetNumberChannels(); ChanNum++) {
      pif.ReadHistogram(histo, HIS_SIZE, ModNum, ChanNum);
      
      unsigned short nhis = 100 * (ModNum + 1) + ChanNum;
      HBOOK1(nhis, hisName, HIS_SIZE, 0, HIS_SIZE, 0);
      for (size_t chan = 0; chan < HIS_SIZE; chan++) {
	HF1(nhis, chan, histo[chan]);
      }
    }
  }

  char fileOptions[] = "N";

  HRPUT(0, hisFile, fileOptions);
  
  return EXIT_SUCCESS;
}
