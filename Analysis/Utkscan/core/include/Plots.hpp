/** \file Plots.hpp
 * \brief A container to request a block of plots and plot respective
 * spectra through the plots register
 * @authors D. Miller, K. Miernik, S. V. Paulauskas
 */
#ifndef __PLOTS_HPP_
#define __PLOTS_HPP_

#include <fstream>
#include <string>
#include <map>
#include <set>
#include <string>

#include "Globals.hpp"
#include "HisFile.hpp"
#include "PlotsRegister.hpp"

//! Holds pointers to all Histograms
class Plots {
public:
    /** Default constructor that takes the offset, range, and name of processor
    * \param [in] offset : the offset for the processor
    * \param [in] range : the range for the processor
    * \param [in] name : the name of the processor */
    Plots(int offset, int range, std::string name);

    /** \return the offset for a given processor */
    int GetOffset() { return offset_; }

    /** Prints out the non empty histograms in the analysis
     * \param [in] hislog : the file stream to print to */
    void PrintNonEmpty(std::ofstream &hislog);

    /** \return true if id is within allowed range.
     * \param [in] id : the id to check */
    bool CheckRange(int id) const;

    /** \return true if histogram number id exists.
    * \param [in] id : the id to check */
    bool Exists(int id) const;

    /** \return true if histogram with mnemonic mne exists.
     * \param [in] mne : the name to check in the list */
    bool Exists(const std::string &mne) const;


    /*! \brief Declares a 1D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] title : The title for the histogram
    * \param [in] halfWordsPerChan : the half words per channel in the his
    * \param [in] xHistLength : length of the histogram in the x axis
    * \param [in] xLow : the Low range of the histogram
    * \param [in] xHigh : the High range of the histogram
    * \param [in] mne : the mnemonic for the histogram
    * \return true if things go all right */
    bool DeclareHistogram1D(int dammId, int xSize, const char *title,
                            int halfWordsPerChan, int xHistLength, int xLow,
                            int xHigh,
                            const std::string &mne = "");

    /*! \brief Declares a 1D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] title : The title for the histogram
    * \param [in] halfWordsPerChan : the half words per channel in the his
    * \param [in] mne : the mnemonic for the histogram
    * \return true if things go all right */
    bool DeclareHistogram1D(int dammId, int xSize, const char *title,
                            int halfWordsPerChan = 2,
                            const std::string &mne = "");

    /*! \brief Declares a 1D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] title : The title for the histogram
    * \param [in] halfWordsPerChan : the half words per channel in the his
    * \param [in] contraction : the histogram contraction number
    * \param [in] mne : the mnemonic for the histogram
    * \return true if things go all right */
    bool DeclareHistogram1D(int dammId, int xSize, const char *title,
                            int halfWordsPerChan, int contraction,
                            const std::string &mne = "");

    /*! \brief Declares a 2D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] ySize : The range of the y-axis
    * \param [in] title : The title of the histogram
    * \param [in] halfWordsPerChan : the half words per channel in the his
    * \param [in] xHistLength : length of the histogram in the x axis
    * \param [in] xLow : the Low range of the histogram
    * \param [in] xHigh : the High range of the histogram
    * \param [in] yHistLength : length of the histogram in the y axis
    * \param [in] yLow : the Low for the y-range of the histogram
    * \param [in] yHigh : the High for the y-range of the histogram
    * \param [in] mne : the mnemonic for the histogram
    * \return true if things go all right */
    bool DeclareHistogram2D(int dammId, int xSize, int ySize, const char *title,
                            int halfWordsPerChan, int xHistLength, int xLow,
                            int xHigh, int yHistLength, int yLow, int yHigh,
                            const std::string &mne = "");

    /*! \brief Declares a 2D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] ySize : The range of the y-axis
    * \param [in] title : The title of the histogram
    * \param [in] halfWordPerChan : the half words per channel in the his
    * \param [in] mne : the mnemonic for the histogram
    * \return true if things go all right */
    bool DeclareHistogram2D(int dammId, int xSize, int ySize, const char *title,
                            int halfWordPerChan = 1,
                            const std::string &mne = "");

    /*! \brief Declares a 2D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] ySize : the range of the y-axis
    * \param [in] title : The title for the histogram
    * \param [in] halfWordsPerChan : the half words per channel in the his
    * \param [in] xContraction : the histogram x contraction number
    * \param [in] yContraction : the histogram y contraction number
    * \param [in] mne : the mnemonic for the histogram
    * \return true if things go all right */
    bool DeclareHistogram2D(int dammId, int xSize, int ySize, const char *title,
                            int halfWordsPerChan, int xContraction,
                            int yContraction, const std::string &mne = "");

    /*! \brief Plots into histogram defined by dammId
    * \param [in] dammId : The histogram number to define
    * \param [in] val1 : the x value
    * \param [in] val2 : the y value or weight for a 1D histogram
    * \param [in] val3 : the z value or weight in a 2D histogram
    * \param [in] name : the name of the histogram in ROOT (not implemented now)
    * \return true if successful */
    bool Plot(int dammId, double val1, double val2 = -1, double val3 = -1,
              const char *name = "h");

    /*! \brief Plots into histogram defined by mne
    * \param [in] mne  : the mnemonic to plot into
    * \param [in] val1 : the x value
    * \param [in] val2 : the y value
    * \param [in] val3 : the z value
    * \param [in] name : the name of the histogram
    * \return true if successful */
    bool Plot(const std::string &mne, double val1, double val2 = -1,
              double val3 = -1, const char *name = "h");

    /** Method to test if a parameter is inside of a loaded banana
    *
    * Will not help you defend against a man wielding a pointed stick.
    * \param [in] id : the banana id to look at
    * \param [in] x : the x value to check
    * \param [in] y : the y value to check
    * \return true if the x,y coordinate was inside the banana */
    bool BananaTest(const int &id, const double &x, const double &y);

private:
    static PlotsRegister *plots_register_;//!< Instance of the plots register
    /** Holds offset for a given set of plots */
    int offset_;
    /** Holds allowed range for a given set of plots*/
    int range_;
    /** Name of the owner of plots, mainly for debugging */
    std::string name_;
    /** set of int (relative dammId without offsets */
    std::set<int> idList_;
    /** Map of mnemonic -> int */
    std::map<std::string, int> mneList;
    /** Map of dammid -> title, helps debugging duplicated dammids*/
    std::map<int, std::string> titleList;

    /** A function to round the value before passing it to DAMM
    * \param [in] val : the value to round
    * \return the rounded value */
    int Round(double val) const;
};

#endif // __PLOTS_HPP_
