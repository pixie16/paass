/*! \file Correlator.cpp
 *
 *  The correlator class keeps track of where and when implantation and decay
 *  events have occurred, and then correlates each decay with its corresponding
 *  implant. A decay will only be validate if it occurred close enough in time
 *  to the implant, and the implant was well separated in time with regard to
 *  all other implants at the same location
 *  
 *  This file is derived from previous "correlator.cpp"
 *  
 *  David Miller, April 2010
 */

#include <iomanip>
#include <iostream>

#include <cmath>

#include "damm_plotids.h"
#include "param.h"
#include "RawEvent.h"
#include "Correlator.h"

using namespace std;

const double Correlator::minImpTime = 5e-3 / pixie::clockInSeconds;
const double Correlator::corrTime   = 3300 / pixie::clockInSeconds;
const double Correlator::fastTime   = 8e-6 / pixie::clockInSeconds;

Correlator::Correlator() : 
    lastImplant(NULL), lastDecay(NULL), condition(OTHER_EVENT)
{
}

void Correlator::DeclarePlots() const
{
    using namespace dammIds::correlator;
    static bool done = false;

    if (done)  {
	return;
    }

    DeclareHistogram1D(D_CONDITION, S9, "Correlator condition");
    DeclareHistogram1D(D_TIME_BW_IMPLANTS, S9, "time between implants, 100 ms/bin"); 
    DeclareHistogram1D(D_TIME_BW_ALL_IMPLANTS, SA, "time between all implants, 1 us/bin"); 

    done = true;
}

void Correlator::Correlate(RawEvent &event, EEventType type,
			   unsigned int frontCh, unsigned int backCh,
			   double time, double energy)
{
    using namespace dammIds::correlator;

    if (frontCh < 0 || frontCh >= MAX_STRIP ||
	backCh < 0  || backCh >= MAX_STRIP) {
	plot(D_CONDITION, INVALID_LOCATION);
	return;
    }

    ImplantData &imp = implant[frontCh][backCh];
    DecayData   &dec = decay[frontCh][backCh];

    if (type == IMPLANT_EVENT) {
        decaylist[frontCh][backCh].clear();
	decaylist[frontCh][backCh].push_back( make_pair(energy, time) );

	condition = VALID_IMPLANT;
	if (lastImplant != NULL) {
	    double dt = time - lastImplant->time;
	    plot(D_TIME_BW_ALL_IMPLANTS, dt * pixie::clockInSeconds / 1e-6);
	}
	if (imp.implanted) {
	    condition = BACK_TO_BACK_IMPLANT;
	    imp.dtime = time - imp.time;
	    imp.tacValue = 0;
	    plot(D_TIME_BW_IMPLANTS, imp.dtime * pixie::clockInSeconds / 100e-3 );
	} else {
	    imp.implanted = true;
	    imp.dtime = INFINITY;
	}
	lastImplant = &imp;
	imp.time = time;
    } else if (type == DECAY_EVENT && imp.implanted) {
	condition = VALID_DECAY;
	decaylist[frontCh][backCh].push_back( make_pair(energy,time) );

	if (time < imp.time ) {	 
	    cout << "negative correlation time, DECAY: " << time 
		 << " IMPLANT: " << imp.time 
		 << " DT:" << (time - imp.time) << endl;
	    if (imp.time - time > 5e11 && time < 1e9 ) {
		// PIXIE's clock has most likely been zeroed due to file marker, no chance of doing correlations
		for (unsigned int i=0; i < MAX_STRIP; i++) {
		    for (unsigned int j=0; j < MAX_STRIP; j++) {
			if (implant[i][j].time > time) {
			    implant[i][j].Clear();
			}
		    }
		}
	    }
	} // negative correlation time
	if ( imp.dtime >= minImpTime ) {
	    if (time - imp.time < corrTime) {
		dec.time    = time;
		dec.dtime   = time - imp.time;
		
		lastDecay = &dec;
	    } else {
		condition = DECAY_TOO_LATE;
	    }
	} else {
	    condition = IMPLANT_TOO_SOON;
	}
    } // is decay with implant 
    else {
	condition = UNKNOWN_EVENT;
    }

    plot(D_CONDITION, condition);
}

void Correlator::PrintDecayList(unsigned int fch, unsigned int bch) const
{
    const corrlist_t &l = decaylist[fch][bch];
    const double printTimeResolution = 1e-3;

    if (l.empty()) {
	cout << " Current event list for " << fch << " , " << bch <<  " is empty." << endl;
	return;
    }
    double firstTime = l.at(0).second;
    double lastTime = firstTime;
    
    cout << " Current event list for " << fch << " , " << bch << " : " << endl;
    cout << "    TAC: " << implant[fch][bch].tacValue << endl;
    
    for (corrlist_t::const_iterator it = l.begin(); it != l.end(); it++) {
	double dt  = ((*it).second - firstTime) * pixie::clockInSeconds / printTimeResolution;
	double dt2 = ((*it).second - lastTime)  * pixie::clockInSeconds / printTimeResolution;

	if ( dt2 < fastTime && it != l.begin() ) {
	    cout << "    FAST DECAY!!!" << endl;
	}
	cout << "    E " << setw(10) << fixed << setprecision(3) << (*it).first
	     << " [ch] at T " << setw(10) << dt 
	     << ", DT= " << setw(10) << dt2 << " [ms]" << endl;
	lastTime = (*it).second;
    } 
    cout.unsetf(ios::floatfield);
}
