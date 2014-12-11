/*! \file BarDetector.hpp
 *  \brief A class to handle bar style detectors (VANDLE, Betas, etc.)
 *  \author S. V. Paulauskas
 *  \date November 22, 2014
*/
#ifndef __BARDETECTOR_HPP__
#define __BARDETECTOR_HPP__

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
    BarDetector(const HighResTimingData &Right, const HighResTimingData &Left,
                const TimingCalibration &cal, const std::string &type);

    /** \return the hasEvent_ var */
    bool GetHasEvent() const {return(hasEvent_);};
    /** \return the  var */
    double GetFlightPath() const {return(flightPath_);};
    /** \return the qdc_ var */
    double GetQdc() const {return(qdc_);};
    /** \return the qdcPos_ var */
    double GetQdcPosition() const {return(qdcPos_);};
    /** \return the theta_ var */
    double GetTheta() const {return(theta_);};
    /** \return the timeAve_ var */
    double GetTimeAverage() const {return(timeAve_);};
    /** \return the timeDiff_ var */
    double GetTimeDifference() const {return(timeDiff_);};
    /** \return the walkCorTimeAve_ var */
    double GetWalkCorTimeAve() const {return(walkCorTimeAve_);};
    /** \return the walkCorTimeDiff_ var */
    double GetWalkCorTimeDiff() const {return(walkCorTimeDiff_);};
    /** \return the left_ var */
    HighResTimingData GetLeftSide() const {return(left_);};
    /** \return the right_ var */
    HighResTimingData GetRightSide() const {return(right_);};
    /** \return the type of bar detector */
    std::string GetType() const {return(type_);};
    /** \return The cal_ var */
    TimingCalibration GetCalibration() const {return(cal_);};

private:
    bool hasEvent_; //!< true if there was an event in the bar
    double flightPath_; //!< flight path of the particle to the detector
    double qdc_;//!< QDC for the bar
    double qdcPos_;//!< Position derived from the QDC
    double theta_;//!< the angle of the particle w.r.t. the source
    double timeAve_;//!< The average arrival time of the left and right ends
    double timeDiff_;//!< The time difference between the two ends
    double walkCorTimeDiff_;//!< The walk corrected time difference
    double walkCorTimeAve_;//!< The walk corrected time average
    std::string type_; //!< The type of bar detector it is
    HighResTimingData right_; //!< The Right side of the detector
    HighResTimingData left_; //!< The Left side of the detector
    TimingCalibration cal_; //!< The Timing Calibration of the bar

    /** \brief Checks to see if the event could physically be inside the bar*/
    void BarEventCheck(void);
    /** \brief Calculates the flight path of the particle */
    void CalcFlightPath(void);
};

/** Defines a map to hold Bar Detectors */
typedef std::map<TimingDefs::BarIdentifier, BarDetector> BarMap;
#endif // __BARDETECTOR_HPP__
