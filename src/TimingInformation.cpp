/** \file TimingInformation.cpp
 *\brief Structures and methods for high res timing
 *
 *Contains common data structures and methods needed for high
 *resolution timing analysis
 *
 *\author S. V. Paulauskas
 *\date 09 May 2011
 */
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TimingInformation.hpp"
#include "Trace.hpp"

using namespace std;

Globals *constants = Globals::get();

TimingInformation::TimingCalMap TimingInformation::calibrationMap;

TimingInformation::TimingData::TimingData(void) : trace(emptyTrace) {
    aveBaseline    = numeric_limits<double>::quiet_NaN();
    discrimination = numeric_limits<double>::quiet_NaN();
    highResTime    = numeric_limits<double>::quiet_NaN();
    maxpos         = numeric_limits<double>::quiet_NaN();
    maxval         = numeric_limits<double>::quiet_NaN();
    phase          = numeric_limits<double>::quiet_NaN();
    snr            = numeric_limits<double>::quiet_NaN();
    stdDevBaseline = numeric_limits<double>::quiet_NaN();
    tqdc           = numeric_limits<double>::quiet_NaN();
    walk           = numeric_limits<double>::quiet_NaN();
    walkCorTime    = numeric_limits<double>::quiet_NaN();

    numAboveThresh = -1;
}

TimingInformation::TimingData::TimingData(ChanEvent *chan) :
    trace(chan->GetTrace()) {
    //put all the times as ns
    const Trace& trace = chan->GetTrace();
    aveBaseline    = trace.GetValue("baseline");
    discrimination = trace.GetValue("discrim");
    highResTime    = chan->GetHighResTime()*1e+9;
    maxpos         = trace.GetValue("maxpos");
    maxval         = trace.GetValue("maxval");
    numAboveThresh = trace.GetValue("numAboveThresh");
    phase          = trace.GetValue("phase")*
                        (Globals::get()->clockInSeconds()*1e+9);
    stdDevBaseline = trace.GetValue("sigmaBaseline");
    tqdc           = trace.GetValue("tqdc")/qdcCompression;
    walk           = trace.GetValue("walk");

    //Calculate some useful quantities.
    snr = 20*log10(maxval/stdDevBaseline);
    walkCorTime   = highResTime - walk;

    //validate data and set a flag saying it's ok
    // clean up condition at some point
    if((maxval == maxval) && (phase == phase) &&
       (tqdc == tqdc) && (highResTime == highResTime) &&
       (stdDevBaseline == stdDevBaseline))
	dataValid = true;
    else
	dataValid = false;
}

TimingInformation::BarData::BarData(const TimingData &Right,
                                    const TimingData &Left,
                                    const TimingCal &cal, const string &type) {
    timeOfFlight.clear();
    energy.clear();
    corTimeOfFlight.clear();

    lMaxVal   = Left.maxval;
    rMaxVal   = Right.maxval;
    lqdc      = Left.tqdc;
    rqdc      = Right.tqdc;
    lTime     = Left.highResTime;
    rTime     = Right.highResTime;
    qdc       = sqrt(Right.tqdc*Left.tqdc);
    timeAve   = (Right.highResTime + Left.highResTime)*0.5;
    timeDiff  = (Left.highResTime-Right.highResTime) + cal.lrtOffset;
    walkCorTimeDiff = (Left.walkCorTime-Right.walkCorTime) + cal.lrtOffset;
    walkCorTimeAve  = (Left.walkCorTime+Right.walkCorTime)*0.5;

    event   = BarEventCheck(timeDiff, type);
    flightPath = CalcFlightPath(timeDiff, cal, type);
    theta      = acos(cal.z0/flightPath);
}

bool TimingInformation::BarData::BarEventCheck(const double &timeDiff,
                                               const string &type) {
    if(type == "small") {
	double lengthSmallTime = constants->smallLengthTime();
	return(fabs(timeDiff) < lengthSmallTime+20);
    } else if(type == "big") {
	double lengthBigTime = constants->bigLengthTime();
	return(fabs(timeDiff) < lengthBigTime+20);
    } else
	return(false);
}

double TimingInformation::BarData::CalcFlightPath(double &timeDiff,
                                                  const TimingCal& cal,
                                                  const string &type) {
    if(type == "small")
        return(sqrt(cal.z0*cal.z0+
		    pow(constants->speedOfLightSmall()*0.5*timeDiff+cal.xOffset,2)));
    else if(type == "big")
        return(sqrt(cal.z0*cal.z0 +
 		    pow(constants->speedOfLightBig()*0.5*timeDiff+cal.xOffset,2)));
    else
        return(numeric_limits<double>::quiet_NaN());
}

double TimingInformation::CalcEnergy(const double &corTOF, const double &z0) {
    return((0.5*constants->neutronMass()*
            pow((z0/corTOF)/constants->speedOfLight(), 2))*1000);
}

TimingInformation::TimingCal TimingInformation::GetTimingCal(const IdentKey
                                                             &identity) {
    map<IdentKey, TimingCal>::iterator itTemp =
        calibrationMap.find(identity);
    if(itTemp == calibrationMap.end()) {
        cout << endl << endl
             << "Cannot locate detector named " << identity.second
             << " at location " << identity.first
             << "in the Timing Calibration!!"
             << endl << "Please check timingCal.txt" << endl << endl;
        exit(EXIT_FAILURE);
    } else {
        TimingCal value = (*calibrationMap.find(identity)).second;
        return(value);
    }
}

void TimingInformation::ReadTimingCalibration(void) {
    TimingCal timingcal;

    string timeCalFileName = Globals::get()->configPath("timingCal.txt");

    ifstream timingCalFile(timeCalFileName.c_str());

    if (!timingCalFile) {
        cout << endl << "Cannot open file 'timingCal.txt'"
	     << "-- This is Fatal! Exiting..." << endl << endl;
	exit(EXIT_FAILURE);
    } else {
	while(timingCalFile) {
	    if (isdigit(timingCalFile.peek())) {
		unsigned int location = -1;
		string type = "";

		timingCalFile >> location >> type
			      >> timingcal.z0 //position stuff
			      >> timingcal.xOffset >> timingcal.zOffset
			      >> timingcal.lrtOffset  //time stuff
			      >> timingcal.tofOffset0 >> timingcal.tofOffset1;

		//minimum distance to the bar
		timingcal.z0 += timingcal.zOffset;

		IdentKey calKey(location, type);

		calibrationMap.insert(make_pair(calKey, timingcal));
	    } else{
		timingCalFile.ignore(1000, '\n');
	    }
	} // end while (!timingCalFile) loop
    }
    timingCalFile.close();
}

#ifdef useroot
TimingInformation::DataRoot::DataRoot(void) {
    multiplicity = 0;
    dummy = -1;

    for (size_t i = 0; i < maxMultiplicity; i++) {
        aveBaseline[i]    = numeric_limits<double>::quiet_NaN();
        discrimination[i] = numeric_limits<double>::quiet_NaN();
        highResTime[i]    = numeric_limits<double>::quiet_NaN();
        maxpos[i]         = numeric_limits<double>::quiet_NaN();
        maxval[i]         = numeric_limits<double>::quiet_NaN();
        phase[i]          = numeric_limits<double>::quiet_NaN();
        stdDevBaseline[i] = numeric_limits<double>::quiet_NaN();
        tqdc[i]           = numeric_limits<double>::quiet_NaN();
        location[i]       = -1;
    }
}
#endif
