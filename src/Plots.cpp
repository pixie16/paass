/** \file Plots.cpp
 * \brief Implement a block declaration scheme for DAMM plots
 */

#include <iostream>
#include <sstream>

#include <cmath>
#include <cstring>

#include "Plots.hpp"
#include "PlotsRegister.hpp"
#include "Exceptions.hpp"

using namespace std;

/** Do banana gating using ban files args are the Banana number in the ban file,
 * the x-value to test, and the y-value to test.
 * \return true if the x,y pair is inside the banana gate */
extern "C" bool bantesti_(const int &, const int &, const int &);

/** create a DAMM 1D histogram
 * args are damm id, half-words per channel, param length, hist length,
 * low x-range, high x-range, and title
 */
extern "C" void hd1d_(const int &, const int &, const int &, const int &,
		      const int &, const int &, const char *, int);

/** create a DAMM 2D histogram
 * args are damm id, half-words per channel, x-param length, x-hist length
 * low x-range, high x-range, y-param length, y-hist length, low y-range
 * high y-range, and title
 */
extern "C" void hd2d_(const int &, const int &, const int &, const int &,
		      const int &, const int &, const int &, const int &,
		      const int &, const int &, const char *, int);

Plots::Plots(int offset, int range, std::string name) {
    offset_ = offset;
    range_  = range;
    name_ = name;
    PlotsRegister::get()->Add(offset_, range_, name_);
}

bool Plots::BananaTest(const int &id, const double &x, const double &y) {
    return (bantesti_(id, Round(x), Round(y)));
}

/**
 * Check if the id falls within the expected range
 */
bool Plots::CheckRange(int id) const {
    return (id < range_ && id >= 0);
}

/**
 * Checks if id is taken
 */
bool Plots::Exists(int id) const {
    return (idList.count(id) != 0);
}

bool Plots::Exists(const std::string &mne) const {
    // Empty mnemonic is always allowed
    if (mne.size() == 0)
	return false;

    return (mneList.count(mne) != 0);
}

/** Constructors based on DeclareHistogram functions. */
bool Plots::DeclareHistogram1D(int dammId, int xSize, const char* title,
			       int halfWordsPerChan, int xHistLength,
			       int xLow, int xHigh, const std::string &mne) {
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

    pair<set<int>::iterator, bool> result = idList.insert(dammId);
    if (result.second == false)
        return false;
    // Mnemonic is optional and added only if longer then 0
    if (mne.size() > 0)
        mneList.insert( pair<string, int>(mne, dammId) );
    hd1d_(dammId + offset_, halfWordsPerChan, xSize, xHistLength,
          xLow, xHigh, title, strlen(title));
    titleList.insert( pair<int, string>(dammId, string(title)));
    return true;
}

bool Plots::DeclareHistogram1D(int dammId, int xSize, const char* title,
			       int halfWordsPerChan /* = 2*/,
			       const std::string &mne /*=empty*/ ) {
    return DeclareHistogram1D(dammId, xSize, title, halfWordsPerChan,
                              xSize, 0, xSize - 1, mne);
}

bool Plots::DeclareHistogram1D(int dammId, int xSize, const char* title,
                               int halfWordsPerChan, int contraction,
                               const std::string &mne) {
    return DeclareHistogram1D(dammId, xSize, title, halfWordsPerChan,
			      xSize / contraction, 0, xSize / contraction - 1, mne);
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
                               const char *title, int halfWordsPerChan,
                               int xHistLength, int xLow, int xHigh,
                               int yHistLength, int yLow, int yHigh,
			       const std::string &mne) {
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

    pair<set<int>::iterator, bool> result = idList.insert(dammId);
    if (result.second == false)
        return false;
    // Mnemonic is optional and added only if longer then 0
    if (mne.size() > 0)
        mneList.insert( pair<string, int>(mne, dammId) );

    hd2d_(dammId + offset_, halfWordsPerChan, xSize, xHistLength, xLow, xHigh,
	  ySize, yHistLength, yLow, yHigh, title, strlen(title));
    titleList.insert( pair<int, string>(dammId, string(title)));
    return true;
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
                               const char* title,
                               int halfWordsPerChan /* = 1*/,
                               const std::string &mne /* = empty*/) {
    return DeclareHistogram2D(dammId, xSize, ySize, title, halfWordsPerChan,
			      xSize, 0, xSize - 1,
			      ySize, 0, ySize - 1, mne);
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
			       const char* title, int halfWordsPerChan,
			       int xContraction, int yContraction, const std::string &mne) {
    return DeclareHistogram2D(dammId, xSize, ySize, title, halfWordsPerChan,
			      xSize / xContraction, 0, xSize / xContraction - 1,
			      ySize / yContraction, 0, ySize / yContraction - 1,
			      mne );
}

bool Plots::Plot(int dammId, double val1, double val2, double val3,
                 const char* name) {
    /*
      dammid - id of the damm spectrum in absence of root
      val1   - energy of a 1d spectrum
      x value in a 2d
      val2   - weight in a 1d
      - y value in a 2d
      val3   - weight in a 2d
      name   - name of a root spectrum (NOT CURRENTLY USED)
    */

    /*
    // This checks if dammId is a valid one (fortran code is silent on
    // ploting non-existing plots) but it causes efficiency problems
    // (?) to be tested!
    if (!Exists(dammId))
        return false;
    */

    if (val2 == -1 && val3 == -1)
        count1cc_(dammId + offset_, int(val1), 1);
    else if  (val3 == -1 || val3 == 0)
        count1cc_(dammId + offset_, int(val1), int(val2));
    else
        set2cc_(dammId + offset_, int(val1), int(val2), int(val3));
    return true;
}

bool Plots::Plot(const std::string &mne, double val1, double val2, double val3,
                 const char* name) {
    if (!Exists(mne))
        return false;
    return Plot(mneList.find(mne)->second, val1, val2, val3, name);
}

int Plots::Round(double val) const {
    double intpart;
    if(modf(val, &intpart) < 0.5)
	return(floor(val));
    else
	return(ceil(val));
}
