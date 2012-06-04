#ifndef DAMMPLOTS_H
#define DAMMPLOTS_H

#include <string>
#include <iostream>
#include <map>
#include <set>
#include <utility>
#include <sstream>
#include "PlotsRegister.h"

using namespace std;

/* Fortran subroutines for plotting histograms */
extern "C" void count1cc_(const int &, const int &, const int &);
extern "C" void set2cc_(const int &, const int &, const int &, const int &);


/** Holds pointers to all Histograms.*/
class Plots {
    public:
        Plots (int offset, int range, PlotsRegister* reg) {
            reg->add(offset, range);

            offset_ = offset;
            range_ = range;
        }

        bool checkRange (int id) {
            if (id < range_ && id >= 0)
                return true;
            else 
                return false;
        }

        /** Checks if id is taken */
        bool exists (int id) {
            if (idList_.count(id) == 0)
                return false;
            else
                return true;
        }

        /** Check if mnemonic mne is taken */
        bool exists (const string& mne) {
            // Empty mnemonic is always allowed
            if (mne.size() == 0)
                return false;

            if (mneList_.count(mne) == 0)
                return false;
            else
                return true;
        }

        bool DeclareHistogram1D(int dammId, int xSize, const char* title,
                    int halfWordsPerChan, int xHistLength, int xLow, int xHigh, string mne = "");

        bool DeclareHistogram1D(int dammId, int xSize, const char* title, string mne = "",
                                int halfWordsPerChan = 2); 
        
        bool DeclareHistogram1D(int dammId, int xSize, const char* title,
                    int halfWordsPerChan, int contraction, string mne = "");

        bool DeclareHistogram2D(int dammId, int xSize, int ySize,
                                const char *title, int halfWordsPerChan,
                                int xHistLength, int xLow, int xHigh,
                                int yHistLength, int yLow, int yHigh, string mne = "");

        bool DeclareHistogram2D(int dammId, int xSize, int ySize,
                                const char* title, string mne = "", int halfWordPerChan = 1);

        bool DeclareHistogram2D(int dammId, int xSize, int ySize,
                    const char* title, int halfWordsPerChan,
                    int xContraction, int yContraction, string mne = "");

        bool plot(int dammId, double val1, double val2 = -1, double val3 = -1, const char* name="h") {
        /*
            dammid - id of the damm spectrum in absence of root
            val1   - energy of a 1d spectrum
                    x value in a 2d
            val2   - weight in a 1d
                - y value in a 2d
            val3   - weight in a 2d
            name   - name of a root spectrum
        */
            if (!checkRange(dammId))
                return false;
            if (val2 == -1 && val3 == -1)
                count1cc_(dammId + offset_, int(val1), 1);
            else if  (val3 == -1)
                count1cc_(dammId + offset_, int(val1), int(val2));
            else if ( val3 == 0 )
                count1cc_(dammId + offset_, int(val1), int(val2));
            else 
                set2cc_(dammId + offset_, int(val1), int(val2), int(val3));

            return true;
        }

        bool plot(string mne, double val1, double val2 = -1, double val3 = -1, const char* name="h") {
            if (!exists(mne))
                return false;
            if (plot(mneList_[mne], val1, val2, val3, name))
                return true;
            else
                return false;
        }

    private:
        /** Holds offset for a given set of plots */
        int offset_;
        /** Holds allowed range for a given set of plots*/
        int range_;
        /** set of int (relative dammId without offsets */
        set <int> idList_;
        /** Map of mnemonic -> int */
        map <string, int> mneList_;
};

#endif
