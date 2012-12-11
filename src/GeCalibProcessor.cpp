/** \file GeCalibProcessor.cpp
 *
 * implementation for germanium processor for calibration 
 */

//? Make a clover specific processor

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>

#include <limits>
#include <cmath>
#include <cstdlib>

#include "Plots.hpp"
#include "PlotsRegister.hpp"
#include "DammPlotIds.hpp"

#include "Correlator.hpp"
#include "DetectorLibrary.hpp"
#include "GeProcessor.hpp"
#include "GeCalibProcessor.hpp"
#include "RawEvent.hpp"

using namespace std;

namespace dammIds {
    namespace ge {
        namespace calib {
            const int D_CH_SUM = 400;
            const int D_CH_CRYSTALX = 401;
            const int DD_CH_DETX = 421;
            const int D_ADD_SUM = 450;
            const int D_CH_ADDX = 451;
            const int DD_CH_ADDX = 471;
        }
    }
}

GeCalibProcessor::GeCalibProcessor() : GeProcessor() {
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void GeCalibProcessor::DeclarePlots(void) 
{
    using namespace dammIds::ge;

    DetectorLibrary* modChan = DetectorLibrary::get();
    const set<int> &cloverLocations = modChan->GetLocations("ge",
                                                            "clover_high");
    unsigned int cloverChans = 0;
    for ( set<int>::const_iterator it = cloverLocations.begin();
          it != cloverLocations.end(); it++) {
        leafToClover[*it] = int(cloverChans / 4); 
        cloverChans++;
    }

    if (cloverChans % chansPerClover != 0) {
        cout << " There does not appear to be the proper number of"
            << " channels per clover.\n Program terminating." << endl;
        exit(EXIT_FAILURE);	
    }

    if (cloverChans != 0) {
        numClovers = cloverChans / chansPerClover;
        cout << "A total of " << cloverChans 
             << " clover channels were detected: ";
        int lastClover = numeric_limits<int>::min();
        for ( map<int, int>::const_iterator it = leafToClover.begin();
            it != leafToClover.end(); it++ ) {
            if (it->second != lastClover) {
                lastClover = it->second;
                cout << endl << "  " << lastClover << " : ";
            } else {
                cout << ", ";
            }
                cout << setw(2) << it->first ;
        }

        if (numClovers > MAX_CLOVERS) {
            cout << "This is greater than MAX_CLOVERS for spectra definition."
            << "  Check the spectrum definition file and try again." << endl;
            exit(EXIT_FAILURE);
        }   
    }

    for (unsigned i = 0; i < numClovers; ++i) {
        vector< pair<double, double> > empty;
        addbackEvents_.push_back(empty);
    }

    const int energyBins  = SE;

    DeclareHistogram1D(calib::D_CH_SUM, energyBins, "Ge sum");
    for (unsigned int i = 0; i < cloverChans; i++) {
        stringstream ss;
        ss << "Crystal " << i << " Clover " << leafToClover[i];
        DeclareHistogram1D(calib::D_CH_CRYSTALX + i,
                           energyBins, ss.str().c_str());
    }
    DeclareHistogram2D(calib::DD_CH_DETX, energyBins, S5,
                       "Gamma Ch vs. crystal number ");

    DeclareHistogram1D(calib::D_ADD_SUM, energyBins, "Ge addback sum");
    for (unsigned int i = 0; i < numClovers; i++) {
        stringstream ss;
        ss << " Clover " << i << " addback";
        DeclareHistogram1D(calib::D_CH_ADDX + i,
                           energyBins, ss.str().c_str());
    }
    DeclareHistogram2D(calib::DD_CH_ADDX, energyBins, S3,
                       "Gamma addback Ch vs. clover number ");
}

bool GeCalibProcessor::Process(RawEvent &event) {
    using namespace dammIds::ge;

    if (!EventProcessor::Process(event))
        return false;

    for (vector<ChanEvent*>::iterator it = geEvents_.begin(); 
         it != geEvents_.end(); ++it) {
        ChanEvent *chan = *it;
        double gEnergy = chan->GetCalEnergy();	

        if (gEnergy < 2)
            continue;

        int det = chan->GetChanID().GetLocation();

        plot(calib::D_CH_CRYSTALX + det, gEnergy);
        plot(calib::D_CH_SUM, gEnergy);
        plot(calib::DD_CH_DETX, gEnergy, det);
    }

    unsigned nEvents = addbackEvents_[0].size();
    for (unsigned int ev = 0; ev < nEvents; ++ev) {
        for (unsigned int det = 0; det < numClovers; ++det) {
            double gEnergy = addbackEvents_[det][ev].first;
            if (gEnergy < 2)
                continue;

            plot(calib::D_CH_ADDX + det, gEnergy);
            plot(calib::D_ADD_SUM, gEnergy);
            plot(calib::DD_CH_ADDX, gEnergy, det);
        }
    }

    EndProcess(); 
    return true;
}
