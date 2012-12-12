/** \file Plots.hpp
 * \brief A container to request a block of plots and plot respective 
 * spectra through the plots register
 */

#ifndef __PLOTS_HPP_
#define __PLOTS_HPP_

#include <fstream>
#include <string>
#include <map>
#include <set>
#include "PlotsRegister.hpp"

#include "Globals.hpp"

/* Fortran subroutines for plotting histograms */
extern "C" bool bantesti_(const int &, const int &, const int &);
extern "C" void count1cc_(const int &, const int &, const int &);
extern "C" void set2cc_(const int &, const int &, const int &, const int &);

/** Holds pointers to all Histograms.*/
class Plots {
public:
    Plots (int offset, int range);

    int GetOffset() { return offset_; }

    void PrintNonEmpty(std::ofstream& hislog);

    /**Returns true if id is within allowed range.*/
    bool CheckRange (int id) const;

    /** Returns true if histogram number id exists.*/
    bool Exists (int id) const;

    /** Returns true if histogram with mnemonic mne exists.*/
    bool Exists (const std::string &mne) const;

    bool DeclareHistogram1D(int dammId, int xSize, const char* title,
			    int halfWordsPerChan, int xHistLength, int xLow, 
                            int xHigh, const std::string &mne = emptyString);
    bool DeclareHistogram1D(int dammId, int xSize, const char* title,
                            int halfWordsPerChan = 2, 
                            const std::string &mne = emptyString); 
    bool DeclareHistogram1D(int dammId, int xSize, const char* title,
			    int halfWordsPerChan, int contraction,
			    const std::string &mne = emptyString);

    bool DeclareHistogram2D(int dammId, int xSize, int ySize,
			    const char *title, int halfWordsPerChan,
			    int xHistLength, int xLow, int xHigh,
			    int yHistLength, int yLow, int yHigh,
			    const std::string &mne = emptyString);    
    bool DeclareHistogram2D(int dammId, int xSize, int ySize,
			    const char* title, int halfWordPerChan = 1,
                            const std::string &mne = emptyString);
    bool DeclareHistogram2D(int dammId, int xSize, int ySize,
			    const char* title, int halfWordsPerChan,
			    int xContraction, int yContraction, 
			    const std::string &mne = emptyString);
    
    bool Plot(int dammId, double val1, double val2 = -1, double val3 = -1, 
              const char* name="h");
    bool Plot(const std::string &mne, double val1, double val2 = -1, 
              double val3 = -1, const char* name="h");

    bool BananaTest(const int &id, const double &x, const double &y);

private:
    static PlotsRegister* plots_register_;
    /** Holds offset for a given set of plots */
    int offset_;
    /** Holds allowed range for a given set of plots*/
    int range_;
    /** set of int (relative dammId without offsets */
    std::set <int> idList;
    /** Map of mnemonic -> int */
    std::map <std::string, int> mneList;
    /** Map of dammid -> title, helps debugging duplicated dammids*/
    std::map <int, std::string> titleList;
    int Round(double val) const;    
};

#endif // __PLOTS_HPP_
