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

Globals *constants = Globals::get(); //!< Instance of Globals class

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
    tqdc           = trace.GetValue("tqdc")/Globals::get()->qdcCompression();
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
                                    const TimingCalibration &cal,
                                    const std::string &type) {
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
    timeDiff  = (Left.highResTime-Right.highResTime) +
                cal.GetLeftRightTimeOffset();
    walkCorTimeDiff = (Left.walkCorTime-Right.walkCorTime) +
                       cal.GetLeftRightTimeOffset();
    walkCorTimeAve  = (Left.walkCorTime+Right.walkCorTime)*0.5;

    event   = BarEventCheck(timeDiff, type);
    flightPath = CalcFlightPath(timeDiff, cal, type);
    theta      = acos(cal.GetZ0()/flightPath);
}

bool TimingInformation::BarData::BarEventCheck(const double &timeDiff,
                                               const std::string &type) {
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
                                                  const TimingCalibration& cal,
                                                  const std::string &type) {
    if(type == "small")
        return(sqrt(cal.GetZ0()*cal.GetZ0()+
		    pow(constants->speedOfLightSmall()*0.5*timeDiff+cal.GetXOffset(),2)));
    else if(type == "big")
        return(sqrt(cal.GetZ0()*cal.GetZ0() +
 		    pow(constants->speedOfLightBig()*0.5*timeDiff+cal.GetXOffset(),2)));
    else
        return(numeric_limits<double>::quiet_NaN());
}

double TimingInformation::CalcEnergy(const double &corTOF, const double &z0) {
    return((0.5*constants->neutronMass()*
            pow((z0/corTOF)/constants->speedOfLight(), 2))*1000);
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
