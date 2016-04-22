/*! \file BarDetector.hpp
 *  \brief A class to handle bar style detectors (VANDLE, Betas, etc.)
 *  \author S. V. Paulauskas
 *  \date November 22, 2014
*/
#ifndef __BARDETECTOR_HPP__
#define __BARDETECTOR_HPP__

#include <limits>
#include <map>

#include "HighResTimingData.hpp"
#include "TimingCalibrator.hpp"

//! A class to handle detectors that have two readouts viewing the same volume
class BarDetector {
public:
    /** Default constructor */
    BarDetector() {};
    /** Default destructor */
    ~BarDetector() {};

    /** \brief The constructor for the structure
    * \param [in] Right : The right side of the bar
    * \param [in] Left : The left side of the bar
    * \param [in] cal : The timing calibration for the bar
    * \param [in] type : The type of bar that we have */
    BarDetector(const HighResTimingData &Left, const HighResTimingData &Right,
                const TimingCalibration &cal, const std::string &type) {
	left_ = &Left;
	right_ = &Right;
	cal_ = &cal;
	type_ = type;
    }

    /** \return the true if there was an event in the bar */
    bool GetHasEvent() const {
	if(type_ == "small") {
	    double lengthSmallTime = Globals::get()->smallLengthTime();
	    return(fabs(GetTimeDifference()) < lengthSmallTime+20);
	} else if(type_ == "big") {
	    double lengthBigTime = Globals::get()->bigLengthTime();
	    return(fabs(GetTimeDifference()) < lengthBigTime+20);
	} else if (type_ == "medium") {
	    double lengthMediumTime = Globals::get()->mediumLengthTime();
	    return(fabs(GetTimeDifference()) < lengthMediumTime+20);
	}
	return(false);
    }
    /** \return the flight path of the particle to the detector */
    double GetFlightPath() const {
	if(type_ == "small")
	    return(sqrt(cal_->GetZ0()*cal_->GetZ0()+
			pow(Globals::get()->speedOfLightSmall()*0.5*GetTimeDifference()+
			    cal_->GetXOffset(),2)));
	else if(type_ == "big")
	    return(sqrt(cal_->GetZ0()*cal_->GetZ0() +
			pow(Globals::get()->speedOfLightBig()*0.5*GetTimeDifference()+
			    cal_->GetXOffset(),2)));
	else if(type_ == "medium")
	    return(sqrt(cal_->GetZ0()*cal_->GetZ0() +
			pow(Globals::get()->speedOfLightMedium()*0.5*GetTimeDifference()+
			    cal_->GetXOffset(),2)));
	return(std::numeric_limits<double>::quiet_NaN());
    }
    /** \return the position independent qdc for the bar */
    double GetQdc() const {
	return(sqrt(right_->GetTraceQdc()*left_->GetTraceQdc()));
    }
    /** \return the Position derived from the QDC */
    double GetQdcPosition() const {
	return( (left_->GetTraceQdc()-right_->GetTraceQdc()) / 
		(left_->GetTraceQdc() + right_->GetTraceQdc()) );
    }
    /** \return the angle of the particle w.r.t. the source */
    double GetTheta() const {return(acos(cal_->GetZ0()/GetFlightPath()));}
    /** \return The average arrival time of the left and right ends */
    double GetTimeAverage() const {
	return((right_->GetHighResTime() + left_->GetHighResTime())*0.5);
    }
    /** \return the timeDiff_ var */
    double GetTimeDifference() const {
	return((left_->GetHighResTime()-right_->GetHighResTime()) +
                  cal_->GetLeftRightTimeOffset());}
    /** \return The walk corrected time average */
    double GetWalkCorTimeAve() const {
	return(left_->GetWalkCorrectedTime() +
	       right_->GetWalkCorrectedTime()*0.5);
    }
    /** \return the The walk corrected time difference */
    double GetWalkCorTimeDiff() const {
	return(left_->GetWalkCorrectedTime() -
	       right_->GetWalkCorrectedTime() +
	       cal_->GetLeftRightTimeOffset());
    }
    /** \return the left_ var */
    const HighResTimingData* GetLeftSide() const {return(left_);}
    /** \return the right_ var */
    const HighResTimingData* GetRightSide() const {return(right_);}
    /** \return the type of bar detector */
    std::string GetType() const {return(type_);}
    /** \return The cal_ var */
    const TimingCalibration* GetCalibration() const {return(cal_);}
private:
    std::string type_; //!< The type of bar detector it is
    const HighResTimingData *right_; //!< The Right side of the detector
    const HighResTimingData *left_; //!< The Left side of the detector
    const TimingCalibration *cal_; //!< The Timing Calibration of the bar
};

/** Defines a map to hold Bar Detectors */
typedef std::map<TimingDefs::TimingIdentifier, BarDetector> BarMap;
#endif // __BARDETECTOR_HPP__
