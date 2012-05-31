#include <cstring>
#include "DammPlot.h"

using namespace std;

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

/* Fortran subroutines for plotting histograms */
extern "C" void count1cc_(const int &, const int &, const int &);
extern "C" void set2cc_(const int &, const int &, const int &, const int &);

/** Constructors based on DeclareHistogram functions. */
DammPlot::DammPlot(int dammId, int xSize, const char* title,
            int halfWordsPerChan, int xHistLength,
            int xLow, int xHigh) : id_(dammId), dim_(1) {
    hd1d_(dammId, halfWordsPerChan, xSize, xHistLength, xLow, xHigh, title, strlen(title));
}

DammPlot::DammPlot(int dammId, int xSize, const char* title,
            int halfWordsPerChan /* = 2*/) : id_(dammId), dim_(1) {
    hd1d_(dammId, halfWordsPerChan, xSize, xSize, 0, xSize - 1, title, strlen(title));
}

DammPlot::DammPlot(int dammId, int xSize, const char* title,
                   int halfWordsPerChan, int contraction) : id_(dammId), dim_(1) {
    hd1d_(dammId, halfWordsPerChan, xSize, xSize / contraction, 0, xSize / contraction - 1, title, strlen(title));
}
DammPlot::DammPlot(int dammId, int xSize, int ySize,
                   const char *title, int halfWordsPerChan,
                   int xHistLength, int xLow, int xHigh,
                   int yHistLength, int yLow, 
                   int yHigh) : id_(dammId), dim_(2) {
    hd2d_(dammId, halfWordsPerChan, xSize, xHistLength, xLow, xHigh,
          ySize, yHistLength, yLow, yHigh, title, strlen(title));
}
DammPlot::DammPlot(int dammId, int xSize, int ySize,
                   const char* title,
                   int halfWordsPerChan /* = 1*/) : id_(dammId), dim_(2) {
    hd2d_(dammId, halfWordsPerChan, xSize, xSize, 0, xSize - 1,
	      ySize, ySize, 0, ySize - 1, title, strlen(title));
}
DammPlot::DammPlot(int dammId, int xSize, int ySize,
                   const char* title, int halfWordsPerChan,
                   int xContraction, int yContraction) : id_(dammId), dim_(2) {
    hd2d_(dammId, halfWordsPerChan, xSize, xSize / xContraction, 0, xSize / xContraction - 1,
	      ySize, ySize / yContraction, 0, ySize / yContraction - 1, title, strlen(title));
}

/*!
  The plot function is a wrapper around the DAMM plotting routines.  This
  was done for a variety of reasons:
  - enables one command to be used for all plotting
  - allows the mechanism of plotting to be altered at any time without
      affecting the code
  - and allows for a smooth transition to the creation of ROOT histograms.
*/
void Plots::plot(double val1, double val2, double val3, const char *name) {
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
      count1cc_(id_, int(val1), 1);
  else if  (val3 == -1)
      count1cc_(id_, int(val1), int(val2));
  else if ( val3 == 0 )
      count1cc_(id_, int(val1), int(val2));
  else 
      set2cc_(id_, int(val1), int(val2), int(val3));
}
