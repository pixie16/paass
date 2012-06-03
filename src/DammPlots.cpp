#include <cstring>
#include "DammPlots.h"

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
bool Plots::DeclareHistogram1D(int dammId, int xSize, const char* title,
            int halfWordsPerChan, int xHistLength,
            int xLow, int xHigh, string mne) {
    cout << "Declaring: " << dammId << " + " << offset_ << " = " << dammId + offset_ << endl;
    if (!checkRange(dammId)) {
        cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
        exit(1);
        return false;
    }
    if (!exists(dammId) && !exists(mne)) {
        pair<set<int>::iterator, bool> result = idList_.insert(dammId);
        if (result.second == false)
            return false;
        // Mnemonic is optional and added only if longer then 0
        if (mne.size() > 0)
            mneList_.insert( pair<string, int>(mne, dammId) );

        hd1d_(dammId + offset_, halfWordsPerChan, xSize, xHistLength, xLow, xHigh, title, strlen(title));
        return true;
    } else {
        cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
        exit(1);
        return false;
    }
}

bool Plots::DeclareHistogram1D(int dammId, int xSize, const char* title, string mne,
                               int halfWordsPerChan /* = 2*/) {
    cout << "Declaring: " << dammId << " + " << offset_ << " = " << dammId + offset_ << endl;
    if (checkRange(dammId)) {
        cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
        exit(1);
        return false;
    }
    if (!exists(dammId) && !exists(mne)) {
        pair<set<int>::iterator, bool> result = idList_.insert(dammId);
        if (result.second == false)
            return false;
        // Mnemonic is optional and added only if longer then 0
        if (mne.size() > 0)
            mneList_.insert( pair<string, int>(mne, dammId) );

        hd1d_(dammId, halfWordsPerChan, xSize, xSize, 0, xSize - 1, title, strlen(title));
        return true;
    } else {
        cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
        exit(1);
        return false;
    }
}

bool Plots::DeclareHistogram1D(int dammId, int xSize, const char* title,
                               int halfWordsPerChan, int contraction, string mne) {
    cout << "Declaring: " << dammId << " + " << offset_ << " = " << dammId + offset_ << endl;
    if (checkRange(dammId)) {
        cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
        exit(1);
        return false;
    }
    if (!exists(dammId) && !exists(mne)) {
        pair<set<int>::iterator, bool> result = idList_.insert(dammId);
        if (result.second == false)
            return false;
        // Mnemonic is optional and added only if longer then 0
        if (mne.size() > 0)
            mneList_.insert( pair<string, int>(mne, dammId) );

        hd1d_(dammId, halfWordsPerChan, xSize, xSize / contraction, 0, xSize / contraction - 1, title, strlen(title));
        return true;
    } else {
        cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
        exit(1);
        return false;
    }
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
                               const char *title, int halfWordsPerChan,
                               int xHistLength, int xLow, int xHigh,
                               int yHistLength, int yLow, int yHigh, string mne) {
    cout << "Declaring: " << dammId << " + " << offset_ << " = " << dammId + offset_ << endl;
    if (checkRange(dammId)) {
        cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
        exit(1);
        return false;
    }
    if (!exists(dammId) && !exists(mne)) {
        pair<set<int>::iterator, bool> result = idList_.insert(dammId);
        if (result.second == false)
            return false;
        // Mnemonic is optional and added only if longer then 0
        if (mne.size() > 0)
            mneList_.insert( pair<string, int>(mne, dammId) );

        hd2d_(dammId, halfWordsPerChan, xSize, xHistLength, xLow, xHigh,
            ySize, yHistLength, yLow, yHigh, title, strlen(title));
        return true;
    } else {
        cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
        exit(1);
        return false;
    }
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
                               const char* title, string mne, int halfWordsPerChan /* = 1*/) {
    cout << "Declaring: " << dammId << " + " << offset_ << " = " << dammId + offset_ << endl;
    if (checkRange(dammId)) {
        cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
        exit(1);
        return false;
    }
    if (!exists(dammId) && !exists(mne)) {
        pair<set<int>::iterator, bool> result = idList_.insert(dammId);
        if (result.second == false)
            return false;
        // Mnemonic is optional and added only if longer then 0
        if (mne.size() > 0)
            mneList_.insert( pair<string, int>(mne, dammId) );

        hd2d_(dammId, halfWordsPerChan, xSize, xSize, 0, xSize - 1,
            ySize, ySize, 0, ySize - 1, title, strlen(title));
        return true;
    } else {
        cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
        exit(1);
        return false;
    }
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
                   const char* title, int halfWordsPerChan,
                   int xContraction, int yContraction, string mne) {
    cout << "Declaring: " << dammId << " + " << offset_ << " = " << dammId + offset_ << endl;
    if (checkRange(dammId)) {
        cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
        exit(1);
        return false;
    }
    if (!exists(dammId) && !exists(mne)) {
        pair<set<int>::iterator, bool> result = idList_.insert(dammId);
        if (result.second == false)
            return false;
        // Mnemonic is optional and added only if longer then 0
        if (mne.size() > 0)
            mneList_.insert( pair<string, int>(mne, dammId) );

        hd2d_(dammId, halfWordsPerChan, xSize, xSize / xContraction, 0, xSize / xContraction - 1,
            ySize, ySize / yContraction, 0, ySize / yContraction - 1, title, strlen(title));
        return true;
    } else {
        cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
        exit(1);
        return false;
    }
}

bool Plots::plot(int dammId, double val1, double val2, double val3, const char* name) {
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

bool Plots::plot(string mne, double val1, double val2, double val3, const char* name) {
    if (!exists(mne))
        return false;
    if (plot(mneList_[mne], val1, val2, val3, name))
        return true;
    else
        return false;
}
