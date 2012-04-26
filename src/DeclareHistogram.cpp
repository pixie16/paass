/*! \file DeclareHistogram.cpp
 * 
 * C++ wrapper to upak's hd1d/hd2d functions
 * David Miller, Aug. 2009
 */

#include <string>

#include <cstring>

#include "DetectorDriver.h"
#include "damm_plotids.h"

using namespace std;

const int S1 = 2, S2 = 4, S3 = 8, S4 = 16, S5 = 32, S6 = 64, S7 = 128,
    S8 = 256, S9 = 512, SA = 1024, SB = 2048, SC = 4096,
    SD = 8192, SE = 16384, SF = 32768;

/* create a DAMM 1D histogram
 * args are damm id, half-words per channel, param length, hist length,
 * low x-range, high x-range, and title
 */
extern "C" void hd1d_(const int &, const int &, const int &, const int &,
		      const int &, const int &, const char *, int);

/* create a DAMM 2D histogram
 * args are damm id, half-words per channel, x-param length, x-hist length
 * low x-range, high x-range, y-param length, y-hist length, low y-range
 * high y-range, and title
 */
extern "C" void hd2d_(const int &, const int &, const int &, const int &,
		      const int &, const int &, const int &, const int &,
		      const int &, const int &, const char *, int);

// DAMM initialization call
extern "C" void drrmake_();
// DAMM declaration wrap-up call
extern "C" void endrr_();

// c++ wrapper function for hd* calls
void DeclareHistogram1D(int dammId, int xSize, const char* title, int halfWordsPerChan,
			int xHistLength, int xLow, int xHigh)
{    
    hd1d_(dammId, halfWordsPerChan, xSize, xHistLength, xLow, xHigh, title, strlen(title));
}

void DeclareHistogram1D(int dammId, int xSize, const char * title, int halfWordsPerChan)
{
    DeclareHistogram1D(dammId, xSize, title, halfWordsPerChan, xSize, 0, xSize - 1);
}

void DeclareHistogram1D(int dammId, int xSize, const char* title,
			int halfWordsPerChan, int contraction)
{
    DeclareHistogram1D(dammId, xSize, title, halfWordsPerChan,
		       xSize / contraction, 0, xSize / contraction - 1);
}

void DeclareHistogram2D(int dammId, int xSize, int ySize, const char *title, 
			int halfWordsPerChan, int xHistLength, int xLow, int xHigh,
			int yHistLength, int yLow, int yHigh)
{
    hd2d_(dammId, halfWordsPerChan, xSize, xHistLength, xLow, xHigh,
	  ySize, yHistLength, yLow, yHigh, title, strlen(title));
}

void DeclareHistogram2D(int dammId, int xSize, int ySize, 
			const char *title, int halfWordsPerChan )
{
    DeclareHistogram2D(dammId, xSize, ySize, title, halfWordsPerChan,
		       xSize, 0, xSize - 1, ySize, 0, ySize - 1);
}

void DeclareHistogram2D(int dammId, int xSize, int ySize,
			const char* title, int halfWordsPerChan,
			int xContraction, int yContraction)
{
    DeclareHistogram2D(dammId, xSize, ySize, title, halfWordsPerChan,
		       xSize / xContraction, 0, xSize / xContraction - 1,
		       ySize / yContraction, 0, ySize / yContraction - 1);
}

/*! This function defines the histograms to be used in the analysis */
extern "C" void drrsub_(unsigned int& iexist)
{
    using namespace dammIds::misc;

    extern DetectorDriver driver;

    drrmake_(); // initialize things

    driver.DeclarePlots();

    DeclareHistogram1D(D_HIT_SPECTRUM, S7, "channel hit spectrum");
    DeclareHistogram1D(D_SUBEVENT_GAP, SE, "time btwn chan-in event,10ns bin");
    DeclareHistogram1D(D_EVENT_LENGTH, SE, "time length of event, 10 ns bin");
    DeclareHistogram1D(D_EVENT_GAP, SE, "time between events, 10 ns bin");
    DeclareHistogram1D(D_EVENT_MULTIPLICITY, S7, "number of channels in event");
    DeclareHistogram1D(D_BUFFER_END_TIME, SE, "length of buffer, 1 ms bin");
    DeclareHistogram2D(DD_RUNTIME_SEC, SE, S6, "run time - s");
    DeclareHistogram2D(DD_DEAD_TIME_CUMUL, SE, S6, "dead time - cumul");
    DeclareHistogram2D(DD_BUFFER_START_TIME, SE, S6, "dead time - 0.1%");
    DeclareHistogram2D(DD_RUNTIME_MSEC, SE, S7, "run time - ms");
    DeclareHistogram1D(D_NUMBER_OF_EVENTS, S4, "event counter");
    
    endrr_(); // wrap things up
}

/*!
  C++ plotting wrapper

  The plot function is a wrapper around the DAMM plotting routines.  This
  was done for a variety of reasons:
  - enables one command to be used for all plotting
  - allows the mechanism of plotting to be altered at any time without
      affecting the code
  - and allows for a smooth transition to the creation of ROOT histograms.
*/
void plot(int dammID, double val1, double val2, double val3, const char *name)
{
  /*
    dammid - id of the damm spectrum in absence of root
    val1   - energy of a 1d spectrum
             x value in a 2d
    val2   - weight in a 1d
           - y value in a 2d
    val3   - weight in a 2d
    name   - name of a root spectrum
  */

  if (val2 == -1 && val3 == -1)
      count1cc_(dammID,int(val1),1);
  else if  (val3 == -1)
      count1cc_(dammID,int(val1),int(val2));
  else if ( val3 == 0 )
      count1cc_(dammID,int(val1),int(val2));
  else 
      set2cc_(dammID,int(val1),int(val2),int(val3));
}
