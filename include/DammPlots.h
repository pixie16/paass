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

        bool plot(int dammId, double val1, double val2 = -1, double val3 = -1, const char* name="h");

        bool plot(string mne, double val1, double val2 = -1, double val3 = -1, const char* name="h");

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
