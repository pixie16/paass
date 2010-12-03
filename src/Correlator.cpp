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


#include <iostream>

#include <cmath>

#include "damm_plotids.h"
#include "param.h"
#include "RawEvent.h"
#include "Correlator.h"

using std::cout;
using std::endl;


const double Correlator::minImpTime = 5e-3 / pixie::clockInSeconds;
const double Correlator::corrTime   = 3300 / pixie::clockInSeconds;

Correlator::Correlator() : 
    lastImplant(NULL), lastDecay(NULL), condition(OTHER_EVENT)
{
}

void Correlator::DeclarePlots() const
{
    using namespace dammIds::correlator;

    static bool done = false;

    if (done) return;
    
    DeclareHistogram1D(D_CONDITION, S9, "Correlator condition");
    DeclareHistogram1D(D_TIME_BW_IMPLANTS, S9, "time between implants, 100 ms/bin"); 

    done = true;
}

void Correlator::Correlate(RawEvent &event, EEventType type,
			   unsigned int frontCh, unsigned int backCh,
			   double time)
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
	condition = VALID_IMPLANT;
	if (imp.implanted) {
	    condition = BACK_TO_BACK_IMPLANT;
	    imp.dtime = time - imp.time;
	    plot(D_TIME_BW_IMPLANTS, imp.dtime * pixie::clockInSeconds / 100e-3 );
	} else {
	    imp.implanted = true;
	    imp.dtime = INFINITY;

	    lastImplant = &imp;
	}
	imp.time = time;
    } else if (type == DECAY_EVENT) {
	condition = VALID_DECAY;

	if (time < imp.time ) {
	    cout << "negative correlation time" << endl;
	}
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
    } else {
	condition = UNKNOWN_EVENT;
    }

    plot(D_CONDITION, condition);
}

