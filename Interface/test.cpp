// short test program for PixieInterface
#include <cstdlib>

#include "Display.h"
#include "PixieInterface.h"
#include "unistd.h"

int main(int argc, char **argv)
{
  Display::SetColorTerm();
  PixieInterface pif("test.cfg");

  pif.GetSlots();
  pif.Init();

  sleep(20);

  return EXIT_SUCCESS;
}
