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
#include <string>

#include <cmath>
#include <ctime>

#include "damm_plotids.h"
#include "param.h"
#include "DetectorDriver.h"
#include "LogicProcessor.h"
#include "RawEvent.h"
#include "Correlator.h"

using namespace std;

const double Correlator::minImpTime = 5e-3 / pixie::clockInSeconds;
const double Correlator::corrTime   = 3300 / pixie::clockInSeconds;
const double Correlator::fastTime   = 40e-6 / pixie::clockInSeconds;

ListData::ListData(double t, double e, LogicProcessor *lp) : time(t), energy(e)
{
    logicBits[3] = '\0';

    if (lp) {       
	if (!lp->LogicStatus(3)) {
	    logicBits[0]='0'; 
	    offTime = lp->TimeOff(3, time);
	} else logicBits[0]='1'; 
	if (!lp->LogicStatus(4)) {
	    logicBits[1]='0';
	    if (logicBits[0] == '1')
	 	offTime = lp->TimeOff(4, time) + 300e-6 / pixie::clockInSeconds;
	} else logicBits[1]='1';
	if (!lp->LogicStatus(5)) {
	    logicBits[2]='0';
	    if (logicBits[0] == '1' && logicBits[1] == '1') 
		offTime = lp->TimeOff(5, time) + 600e-6 / pixie::clockInSeconds;
	} else logicBits[2]='1';

	clockCount = lp->StopCount(2);
    } else {
	logicBits[0] = logicBits[1] = logicBits[2] = 'X'; // NO LOGIC
	offTime = 0.;
	clockCount = 0;
    }
}

Correlator::Correlator() : 
    lastImplant(NULL), lastDecay(NULL), condition(OTHER_EVENT)
{
}

void Correlator::Init()
{
    // go find a logic processor
    //   strange place to do it but it is only done once and the processor will exist
    extern DetectorDriver driver;
    
    logicProc = NULL;
    vector<EventProcessor*> procs = driver.GetProcessors("logic");
    
    if ( !procs.empty() ) {
	for (vector<EventProcessor*>::iterator it=procs.begin();
	     it != procs.end(); it++) {
	    string name = (*it)->GetName();
	    if (name == "logic" || name == "triggerlogic") {
		logicProc = reinterpret_cast<LogicProcessor*>(*it);
		cout << "Correlator grabbed processor " << name << endl;
		break;
	    }
	}
    }
}

Correlator::~Correlator()
{
    // dump any flagged decay lists which have not been output
    for (unsigned int i=0; i < MAX_STRIP; i++) {
	for (unsigned int j=0; j < MAX_STRIP; j++) {
	    if (implant[i][j].flagged)
		PrintDecayList(i,j);
	}
    }    
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
	if (imp.flagged) {
	    PrintDecayList(frontCh, backCh);
	}
        decaylist[frontCh][backCh].clear();
	decaylist[frontCh][backCh].push_back( ListData(time, energy, logicProc) );

	condition = VALID_IMPLANT;
	if (lastImplant != NULL) {
	    double dt = time - lastImplant->time;
	    plot(D_TIME_BW_ALL_IMPLANTS, dt * pixie::clockInSeconds / 1e-6);
	}
	if (imp.implanted) {
	    condition = BACK_TO_BACK_IMPLANT;
	    imp.dtime = time - imp.time;
	    imp.tacValue = 0;
	    imp.flagged = false;
	    plot(D_TIME_BW_IMPLANTS, imp.dtime * pixie::clockInSeconds / 100e-3 );
	} else {
	    imp.implanted = true;
	    imp.dtime = INFINITY;
	}
	lastImplant = &imp;
	imp.time = time;
    } else if (type == DECAY_EVENT && imp.implanted) {
	condition = VALID_DECAY;
	decaylist[frontCh][backCh].push_back( ListData(time, energy, logicProc) );

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
		double lastTime = NAN;
		if (decaylist[frontCh][backCh].size() == 1) {
		    lastTime = imp.time;
		} else if (decaylist[frontCh][backCh].size() > 1) {
		    lastTime = dec.time;
		}
		double dt = (dec.time - lastTime) * pixie::clockInSeconds;
		if (!isnan(lastTime) && dt< fastTime && dt > 0) {
		    cout << "flagging " << frontCh << " , " << backCh << " in correlator." << endl;
		    Flag(frontCh, backCh);
		}
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
    extern DetectorDriver driver;

    const corrlist_t &l = decaylist[fch][bch];
    const double printTimeResolution = 1e-3;

    if (l.empty()) {
	cout << " Current event list for " << fch << " , " << bch <<  " is empty." << endl;
	return;
    }
    double firstTime = l.at(0).time;
    double lastTime = firstTime;

    time_t theTime = driver.GetWallTime(firstTime);
    
    cout << " " << ctime(&theTime);
    cout << " Current event list for " << fch << " , " << bch << " : " << endl;
    cout << "    TAC: " << setw(8) << implant[fch][bch].tacValue 
	 << ",    ts: " << scientific << setprecision(3) << firstTime 
	 << ",    cc: " << scientific << setprecision(3) << l.at(0).clockCount << endl;
    for (corrlist_t::const_iterator it = l.begin(); it != l.end(); it++) {
	double dt2 = ((*it).time - lastTime);
	if ( dt2 < fastTime && it != l.begin() ) {
	    cout << "    FAST DECAY!!!" << endl;
	}

	double dt   = ((*it).time - firstTime) * pixie::clockInSeconds / printTimeResolution;
	double offt = (*it).offTime * pixie::clockInSeconds / printTimeResolution;

        dt2 *= pixie::clockInSeconds / printTimeResolution;
	
	cout << "    E " << setw(10) << fixed << setprecision(3) << (*it).energy
	     << " [ch] at T " << setw(10) << dt 
	     << ", DT= " << setw(10) << dt2
	     << ", OT(" << (*it).logicBits << ")= " << setw(10) << offt << " [ms]" << endl;	
	lastTime = (*it).time;
    } 
    cout.unsetf(ios::floatfield);
}
