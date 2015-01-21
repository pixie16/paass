/** Program which finds the decay constant for an active pixie channel
 *
 * David Miller, May 2010
 */

#include <iostream>

#include <cstdlib>

#include "utilities.h"

#include "PixieInterface.h"
#include "pixie16app_export.h"

using std::cout;
using std::endl;

class TauFinder : public PixieFunction<>
{
public:
  bool operator()(PixieFunctionParms<> &par);
};

int main(int argc, char **argv)
{
  int mod, ch;
  
  if (argc != 3) {
    cout << "usage: " << argv[0] << " <module> <channel>" << endl;
  }

  mod = atoi(argv[1]);
  ch  = atoi(argv[2]);

  PixieInterface pif("pixie.cfg");

  pif.GetSlots();
  pif.Init();
  pif.Boot(PixieInterface::DownloadParameters |
	   PixieInterface::ProgramFPGA |
	   PixieInterface::SetDAC, true);

  TauFinder finder;

  forChannel<int>(pif, mod, ch, finder);

  return EXIT_SUCCESS;
}

bool TauFinder::operator()(PixieFunctionParms<> &par)
{
  double tau;
  
  int errorNum = Pixie16TauFinder(par.mod, par.ch, &tau);
  cout << "TAU: " << tau << endl;
  cout << "Errno: " << errorNum << endl;

  return (errorNum >= 0);
}
