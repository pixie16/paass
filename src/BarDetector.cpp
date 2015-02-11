/*! \file BarDetector.cpp
 *  \brief A Class to handle bar style detectors (VANDLE, new Betas, etc.)
 *  \author S. V. Paulauskas
 *  \date November 22, 2014
*/
#include <limits>

#include "BarDetector.hpp"

using namespace std;

BarDetector::BarDetector(const HighResTimingData &Left,
                         const HighResTimingData &Right,
                         const TimingCalibration &cal,
                         const std::string &type) {
    left_ = Left;
    right_ = Right;
    cal_ = cal;
    type_ = type;

    timeDiff_  = (Left.GetHighResTime()-Right.GetHighResTime()) +
                  cal.GetLeftRightTimeOffset();

    CalcFlightPath();
    BarEventCheck();

    qdc_       = sqrt(Right.GetTraceQdc()*Left.GetTraceQdc());
    theta_     = acos(cal.GetZ0()/flightPath_);
    timeAve_   = (Right.GetHighResTime() + Left.GetHighResTime())*0.5;
    walkCorTimeDiff_ = (Left.GetWalkCorrectedTime() -
                        Right.GetWalkCorrectedTime()) +
                        cal.GetLeftRightTimeOffset();
    walkCorTimeAve_ = (Left.GetWalkCorrectedTime() +
                       Right.GetWalkCorrectedTime())*0.5;
}

void BarDetector::BarEventCheck() {
    if(type_ == "small") {
        double lengthSmallTime = Globals::get()->smallLengthTime();
        hasEvent_ = fabs(timeDiff_) < lengthSmallTime+20;
    } else if(type_ == "big") {
        double lengthBigTime = Globals::get()->bigLengthTime();
        hasEvent_ = fabs(timeDiff_) < lengthBigTime+20;
    } else if (type_ == "medium") {
        double lengthMediumTime = Globals::get()->mediumLengthTime();
        hasEvent_ = fabs(timeDiff_) < lengthMediumTime+20;
    } else
        hasEvent_ = false;
}

void BarDetector::CalcFlightPath() {
    if(type_ == "small")
        flightPath_ = sqrt(cal_.GetZ0()*cal_.GetZ0()+
                    pow(Globals::get()->speedOfLightSmall()*0.5*timeDiff_+
                        cal_.GetXOffset(),2));
    else if(type_ == "big")
        flightPath_ = sqrt(cal_.GetZ0()*cal_.GetZ0() +
                    pow(Globals::get()->speedOfLightBig()*0.5*timeDiff_+
                        cal_.GetXOffset(),2));
    else if(type_ == "medium")
        flightPath_ = sqrt(cal_.GetZ0()*cal_.GetZ0() +
                    pow(Globals::get()->speedOfLightMedium()*0.5*timeDiff_+
                        cal_.GetXOffset(),2));
    else
        flightPath_ = numeric_limits<double>::quiet_NaN();
}
