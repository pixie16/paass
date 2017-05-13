///@file GeProcessor.cpp
///@brief Implementation for germanium processor
///@author D. Miller, K. Miernik, S. V. Paulauskas
///@date August 2009
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>

#include "DammPlotIds.hpp"
#include "DetectorLibrary.hpp"
#include "Exceptions.hpp"
#include "GeProcessor.hpp"
#include "Messenger.hpp"

namespace dammIds {
    namespace ge {
        const int DD_ENERGY = 0;//!< Energy
    }
}

using namespace std;
using namespace dammIds::ge;

GeProcessor::GeProcessor() : EventProcessor(OFFSET, RANGE, "GeProcessor") {
    associatedTypes.insert("ge"); // associate with germanium detectors
}

void GeProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_ENERGY, SE, S6, "Calibrated Ge Singles");
}

bool GeProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    static const vector<ChanEvent *> &geEvents =
            event.GetSummary("ge", true)->GetList();

    for (vector<ChanEvent *>::const_iterator ge = geEvents.begin();
         ge != geEvents.end(); ge++) {

        if ((*ge)->IsSaturated() || (*ge)->IsPileup())
            continue;

        plot(DD_ENERGY, (*ge)->GetCalibratedEnergy(),
             (*ge)->GetChanID().GetLocation());
    }
    return true;
}