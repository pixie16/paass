/*! \file BarDetector.cpp
 *  \brief A Class to handle bar style detectors (VANDLE, new Betas, etc.)
 *  \author S. V. Paulauskas
 *  \date November 22, 2014
*/
#include <limits>

#include "BarDetector.hpp"

using namespace std;

BarDetector::BarDetector(const HighResTimingData &Right,
                         const HighResTimingData &Left,
                         const TimingCalibration &cal,
                         const std::string &type) {
    qdc_       = sqrt(Right.GetTraceQdc()*Left.GetTraceQdc());
    timeAve_   = (Right.GetHighResTime() + Left.GetHighResTime())*0.5;
    timeDiff_  = (Left.GetHighResTime()-Right.GetHighResTime()) +
                cal.GetLeftRightTimeOffset();
    walkCorTimeDiff_ = (Left.GetWalkCorrectedTime() -
                        Right.GetWalkCorrectedTime()) +
                        cal.GetLeftRightTimeOffset();
    walkCorTimeAve_ = (Left.GetWalkCorrectedTime() +
                       Right.GetWalkCorrectedTime())*0.5;

    hasEvent_ = BarEventCheck(timeDiff_, type);
    flightPath_ = CalcFlightPath(timeDiff_, cal, type);
    theta_ = acos(cal.GetZ0()/flightPath_);
}

bool BarDetector::BarEventCheck(const double &timeDiff,
                                               const std::string &type) {
    if(type == "small") {
        double lengthSmallTime = Globals::get()->smallLengthTime();
        return(fabs(timeDiff) < lengthSmallTime+20);
    } else if(type == "big") {
        double lengthBigTime = Globals::get()->bigLengthTime();
        return(fabs(timeDiff) < lengthBigTime+20);
    } else
        return(false);
}

double BarDetector::CalcFlightPath(double &timeDiff, const TimingCalibration& cal,
                                   const std::string &type) {
    if(type == "small")
        return(sqrt(cal.GetZ0()*cal.GetZ0()+
                    pow(Globals::get()->speedOfLightSmall()*0.5*timeDiff+cal.GetXOffset(),2)));
    else if(type == "big")
        return(sqrt(cal.GetZ0()*cal.GetZ0() +
                    pow(Globals::get()->speedOfLightBig()*0.5*timeDiff+cal.GetXOffset(),2)));
    else
        return(numeric_limits<double>::quiet_NaN());
}
