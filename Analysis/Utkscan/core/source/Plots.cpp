/** \file Plots.cpp
 * \brief Implement a block declaration scheme for DAMM plots
 * @authors D. Miller, K. Miernik, S. V. Paulauskas
 */

#include <iostream>
#include <sstream>

#include <cmath>
#include <cstring>

#include "Plots.hpp"

using namespace std;

Plots::Plots(int offset, int range, std::string name) {
    offset_ = offset;
    range_ = range;
    name_ = name;
    PlotsRegister::get()->Add(offset_, range_, name_);
}

bool Plots::BananaTest(const int &id, const double &x, const double &y) {
    return (bantesti_(id, Round(x), Round(y)));
}

/** Check if the id falls within the expected range */
bool Plots::CheckRange(int id) const {
    return (id < range_ && id >= 0);
}

/** Checks if id is taken */
bool Plots::Exists(int id) const {
    return dammPlotsExist.size() > id && dammPlotsExist[id];
}

bool Plots::Exists(const std::string &mne) const {
    // Empty mnemonic is always allowed
    if (mne.size() == 0)
        return false;
    return (mneList.count(mne) != 0);
}

void Plots::AddHistogram(int dammId, const std::string &mne, const char *title)
{
    // each plots object has an allowed range, if dammId is out of range
    // something is wrong in one of the DetectorDrivers
    if (!CheckRange(dammId)) {
        stringstream ss;
        ss << "Plots: Histogram titled '" << title << "' requests id "
           << dammId << " which is outside of allowed range ("
           << range_ << ") of group with offset (" << offset_ << ").";
        throw HistogramException(ss.str());
    }
    if (Exists(dammId) || Exists(mne)) {
        stringstream ss;
        ss << "Plots: Histogram titled '" << title << "' requests id "
           << dammId + offset_ << " which is already in use by"
           << " histogram '" << titleList[dammId] << "'.";
        throw HistogramException(ss.str());
    }
    // needs to be greater than dammId to prevent segfaulting
    // in the next scope
    if(dammPlotsExist.size() < dammId + 1)
    {
        size_t start = dammPlotsExist.size();
        dammPlotsExist.reserve(dammId + 1);
        for(int i = start; i <= dammId; i++)
        {
            dammPlotsExist.push_back(false);
        }
    }
    dammPlotsExist[dammId] = true;
    if (mne.size() > 0)
        mneList.insert(pair<string, int>(mne, dammId));
    titleList.insert(pair<int, string>(dammId, string(title)));
}


/** Constructors based on DeclareHistogram functions. */
bool Plots::DeclareHistogram1D(int dammId, int xSize, const char *title,
                               int halfWordsPerChan, int xHistLength,
                               int xLow, int xHigh, const std::string &mne) {
    AddHistogram(dammId, mne, title);
    hd1d_(dammId + offset_, halfWordsPerChan, xSize, xHistLength,
          xLow, xHigh, title, strlen(title));
    return true;
}

bool Plots::DeclareHistogram1D(int dammId, int xSize, const char *title,
                               int halfWordsPerChan /* = 2*/,
                               const std::string &mne /*=empty*/ ) {
    return DeclareHistogram1D(dammId, xSize, title, halfWordsPerChan,
                              xSize, 0, xSize - 1, mne);
}

bool Plots::DeclareHistogram1D(int dammId, int xSize, const char *title,
                               int halfWordsPerChan, int contraction,
                               const std::string &mne) {
    return DeclareHistogram1D(dammId, xSize, title, halfWordsPerChan,
                              xSize / contraction, 0, xSize / contraction - 1,
                              mne);
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
                               const char *title, int halfWordsPerChan,
                               int xHistLength, int xLow, int xHigh,
                               int yHistLength, int yLow, int yHigh,
                               const std::string &mne) {
    AddHistogram(dammId, mne, title);
    hd2d_(dammId + offset_, halfWordsPerChan, xSize, xHistLength, xLow, xHigh,
          ySize, yHistLength, yLow, yHigh, title, strlen(title));
    return true;
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
                               const char *title,
                               int halfWordsPerChan /* = 1*/,
                               const std::string &mne /* = empty*/) {
    return DeclareHistogram2D(dammId, xSize, ySize, title, halfWordsPerChan,
                              xSize, 0, xSize - 1,
                              ySize, 0, ySize - 1, mne);
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
                               const char *title, int halfWordsPerChan,
                               int xContraction, int yContraction,
                               const std::string &mne) {
    return DeclareHistogram2D(dammId, xSize, ySize, title, halfWordsPerChan,
                              xSize / xContraction, 0, xSize / xContraction - 1,
                              ySize / yContraction, 0, ySize / yContraction - 1,
                              mne);
}

bool Plots::Plot(int dammId, double val1, double val2, double val3,
                 const char *name) {
    // We will not try to plot into histograms that have not been defined
    if (!Exists(dammId)) {
#ifdef VERBOSE
        std::cerr << "Tried to fill histogram ID " << dammId << "belonging to "
                  << name_ << ", which is not known to us. You MUST fix this "
                  << "before continuing with execution." << endl;
        //We will exit here since this is an error that should be fixed. 
        exit(0);
#endif
        return (false);
    }

    if (val2 == -1 && val3 == -1)
        count1cc_(dammId + offset_, int(val1), 1);
    else if (val3 == -1 || val3 == 0)
        count1cc_(dammId + offset_, int(val1), int(val2));
    else
        set2cc_(dammId + offset_, int(val1), int(val2), int(val3));
    return (true);
}

bool Plots::Plot(const std::string &mne, double val1, double val2, double val3,
                 const char *name) {
    if (!Exists(mne))
        return false;
    return Plot(mneList.find(mne)->second, val1, val2, val3, name);
}

int Plots::Round(double val) const {
    double intpart;
    if (modf(val, &intpart) < 0.5)
        return (floor(val));
    else
        return (ceil(val));
}
