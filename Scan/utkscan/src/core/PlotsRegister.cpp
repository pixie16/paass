/** \file PlotsRegister.cpp
 * \brief Declaration of singleton register
 */

#include <iostream>
#include <sstream>
#include <cstdlib>

#include "PlotsRegister.hpp"
#include "Exceptions.hpp"
#include "Messenger.hpp"

using namespace std;

PlotsRegister* PlotsRegister::instance = NULL;
PlotsRegister* PlotsRegister::get() {
    if (!instance) {
        instance = new PlotsRegister();
    }
    return instance;
}

PlotsRegister::~PlotsRegister() {
    delete instance;
    instance = NULL;
}

bool PlotsRegister::CheckRange (int min, int max) const {
    bool exists = false;

    unsigned sz = reg.size();
    for (unsigned id = 0; id < sz; ++id) {
        if ( (min < reg[id].first && max < reg[id].first) ||
             (min > reg[id].second && max > reg[id].second) )
            continue;
        else {
            exists = true;
            break;
        }
    }
    return exists;
}

bool PlotsRegister::Add (int offset, int range, std::string name) {
    // Special case: empty Plots list
    if (offset == 0 && range == 0)
        return true;

    int min = offset;
    int max = offset + range - 1;

    if (max < min) {
        stringstream ss;
        ss << "PlotsRegister: Attempt to register incorrect "
           << "histogram ids range: "
           << min << " to" << max << " by " << name;
        throw HistogramException(ss.str());
    }

    if (min < 1 || max > 7999) {
        stringstream ss;
        ss << "PlotsRegister: Attempt to register histogram ids: "
           << min << " to " << max << " by " << name << endl;
        ss << "Valid range is 1 to 7999";
        throw HistogramException(ss.str());
    }

    if (CheckRange(min, max)) {
        stringstream ss;
        ss << "PlotsRegister: Attempt to register histogram ids: "
           << min << " to " << max << " by " << name << endl;
        ss << "This range is already registered.";
        throw HistogramException(ss.str());
    }

    reg.push_back( std::pair<int, int>(min, max) );

    Messenger m;
    stringstream ss;
    ss << "Histogram ids: " << min << " to " << max
       << " registered by " << name;
    m.detail(ss.str(), 1);
    return true;
}
