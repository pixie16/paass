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

/** Constructors based on DeclareHistogram functions. */
bool Plots::DeclareHistogram1D(int dammId, int xSize, const char* title,
            int halfWordsPerChan, int xHistLength,
            int xLow, int xHigh, string mne) {
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

        hd1d_(dammId + offset_, halfWordsPerChan, xSize, xSize, 0, xSize - 1, title, strlen(title));
        return true;
    } else {
        cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
        exit(1);
        return false;
    }
}

bool Plots::DeclareHistogram1D(int dammId, int xSize, const char* title,
                               int halfWordsPerChan, int contraction, string mne) {
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

        hd1d_(dammId + offset_, halfWordsPerChan, xSize, xSize / contraction, 0, xSize / contraction - 1, title, strlen(title));
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

        hd2d_(dammId + offset_, halfWordsPerChan, xSize, xHistLength, xLow, xHigh,
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

        hd2d_(dammId + offset_, halfWordsPerChan, xSize, xSize, 0, xSize - 1,
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

        hd2d_(dammId + offset_, halfWordsPerChan, xSize, xSize / xContraction, 0, xSize / xContraction - 1,
            ySize, ySize / yContraction, 0, ySize / yContraction - 1, title, strlen(title));
        return true;
    } else {
        cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
        exit(1);
        return false;
    }
}
