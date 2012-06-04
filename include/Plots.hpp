/** \file Plots.hpp
 * 
 * A container to request a block of plots and plot respective spectrum through the plots register
 */

#ifndef __PLOTS_HPP_
#define __PLOTS_HPP_

#include <string>
#include <map>
#include <set>

#include "Globals.hpp"

// forward declarations
class PlotsRegister;

/* Fortran subroutines for plotting histograms */
extern "C" void count1cc_(const int &, const int &, const int &);
extern "C" void set2cc_(const int &, const int &, const int &, const int &);

/** Holds pointers to all Histograms.*/
class Plots {
public:
    Plots (int offset, int range, PlotsRegister* reg);

    bool CheckRange (int id) const;
    bool Exists (int id) const;
    bool Exists (const std::string &mne) const;

    bool DeclareHistogram1D(int dammId, int xSize, const char* title,
			    int halfWordsPerChan, int xHistLength, int xLow, int xHigh,
			    const std::string &mne = emptyString);
    bool DeclareHistogram1D(int dammId, int xSize, const char* title,
			    const std::string &mne = emptyString,
			    int halfWordsPerChan = 2); 
    bool DeclareHistogram1D(int dammId, int xSize, const char* title,
			    int halfWordsPerChan, int contraction,
			    const std::string &mne = emptyString);

    bool DeclareHistogram2D(int dammId, int xSize, int ySize,
			    const char *title, int halfWordsPerChan,
			    int xHistLength, int xLow, int xHigh,
			    int yHistLength, int yLow, int yHigh,
			    const std::string &mne = emptyString);    
    bool DeclareHistogram2D(int dammId, int xSize, int ySize,
			    const char* title, const std::string &mne = emptyString,
			    int halfWordPerChan = 1);
    bool DeclareHistogram2D(int dammId, int xSize, int ySize,
			    const char* title, int halfWordsPerChan,
			    int xContraction, int yContraction, 
			    const std::string &mne = emptyString);
    
    bool Plot(int dammId, double val1, double val2 = -1, double val3 = -1, const char* name="h") const;

    bool Plot(const std::string &mne, double val1, double val2 = -1, double val3 = -1, const char* name="h") const;

private:
    /** Holds offset for a given set of plots */
    int offset;
    /** Holds allowed range for a given set of plots*/
    int range;
    /** set of int (relative dammId without offsets */
    std::set <int> idList;
    /** Map of mnemonic -> int */
    std::map <std::string, int> mneList;
};

#endif // __PLOTS_HPP_
