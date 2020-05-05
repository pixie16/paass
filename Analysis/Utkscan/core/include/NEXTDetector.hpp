/*! \file NEXTDetector.hpp
 *  \brief A class to handle bar style detectors (VANDLE, Betas, etc.)
 *  \author S. V. Paulauskas
 *  \date November 22, 2014
*/
#ifndef __NEXTDETECTOR_HPP__
#define __NEXTDETECTOR_HPP__

#include <iostream>
#include <limits>
#include <map>

#include "HighResTimingData.hpp"
#include "HighResPositionData.hpp"
#include "TimingCalibrator.hpp"

//! A class to handle detectors that have two readouts viewing the same volume
class NEXTDetector {
public:
    /** Default constructor */
    NEXTDetector() {};

    /** Default destructor */
    ~NEXTDetector() {};

    /** \brief The constructor for the structure
    * \param [in] Right : The right side of the bar
    * \param [in] Left : The left side of the bar
    * \param [in] key : The TimingIdentifier for the bar */
    NEXTDetector(HighResTimingData Left, HighResPositionData aLeft,
                 HighResTimingData Right, HighResPositionData aRight,
                TimingDefs::TimingIdentifier &key) {
        left_ = Left;
        right_ = Right;
        aleft_ = aLeft;
        aright_ = aRight;
        key_ = key;
    }

    /** \return the true if there was an event in the bar */
    bool GetHasEventTiming(void) const {
        /*if (GetType() == "small") {
            double lengthSmallTime = Globals::get()->GetVandleSmallLengthInNs();
            return (fabs(GetTimeDifference()) < lengthSmallTime + 20 &&
                    GetRightSide().GetIsValid() && GetLeftSide().GetIsValid());
        } else if (GetType() == "big") {
            double lengthBigTime = Globals::get()->GetVandleBigLengthInNs();
            return (fabs(GetTimeDifference()) < lengthBigTime + 20 &&
                    GetRightSide().GetIsValid() && GetLeftSide().GetIsValid());
        } else if (GetType() == "medium") {
            double lengthMediumTime =
                    Globals::get()->GetVandleMediumLengthInNs();
            return (fabs(GetTimeDifference()) < lengthMediumTime + 20 &&
                    GetRightSide().GetIsValid() && GetLeftSide().GetIsValid());
        }*/
        return (GetRightSide().GetIsValid() && GetLeftSide().GetIsValid());
    }

    bool GetHasEventPosition(void) const {
        return (GetLeftPos().GetIsValid() && GetRightPos().GetIsValid());
    }

    bool GetHasEvent(void) const{
	return (GetHasEventPosition() && GetHasEventTiming() ) ;
    }

    /** \return the flight path of the particle to the detector */
    double GetFlightPath(void) const {
//        if (GetType() == "small")
            return (sqrt( ( GetCalibration().GetZ0() + GetAverageZPos() * 24.0 ) *
                          ( GetCalibration().GetZ0() + GetAverageZPos() * 24.0 ) +
                         pow(Globals::get()->GetNEXTSpeedOfLightInCmPerNs() *
                             0.5 * GetTimeDifference() +
                             GetCalibration().GetXOffset(), 2) 
                             ));
//        return (std::numeric_limits<double>::quiet_NaN());
    }

    /** \return the position independent qdc for the bar */
    double GetQdc() const {
        return (sqrt(right_.GetTraceQdc() * left_.GetTraceQdc()));
    }
    double GetLpsd() const{
        return left_.GetDiscrimination();
    }
      double GetRpsd() const{
        return right_.GetDiscrimination();
    }
      double Getpsd() const{
        return (sqrt(left_.GetDiscrimination() * right_.GetDiscrimination())) ;
    }
      double GetlMax() const{
          return left_.GetMaximumValue();
      }
       double GetrMax() const{
          return right_.GetMaximumValue();
      }
        double GetlMaxPos() const{
          return left_.GetMaximumPosition();
      }
       double GetrMaxPos() const{
          return right_.GetMaximumPosition();
      }
        double GetrAveBase() const{
          return right_.GetAveBaseline();
      }
        double GetlAveBase() const{
          return left_.GetAveBaseline();
      }
          double GetrSdBase() const{
          return right_.GetStdDevBaseline();
      }
        double GetlSdBase() const{
          return left_.GetStdDevBaseline();
      }

    double GetAnodeQdc() const {
        if ( GetHasEventPosition() )
            return (aleft_.GetTotIntegral() + aright_.GetTotIntegral() );
        else
            return -9999;
    }

    /** \return the Position derived from the QDC */
    double GetQdcPosition() const {
        return ((left_.GetTraceQdc() - right_.GetTraceQdc()) /
                (left_.GetTraceQdc() + right_.GetTraceQdc()));
    }

    /** \return the angle of the particle w.r.t. the source */
    double GetTheta() const {
        return (acos(GetCalibration().GetZ0() / GetFlightPath()));
    }

    /** \return The average arrival time of the left and right ends */
    double GetTimeAverage() const {
        return ((right_.GetHighResTimeInNs() + left_.GetHighResTimeInNs()) *
                0.5);
    }

    /** \return the timeDiff_ var */
    double GetTimeDifference() const {
        return ((left_.GetHighResTimeInNs() - right_.GetHighResTimeInNs()) +
                GetCalibration().GetLeftRightTimeOffset());
    }

    /** \return The walk corrected time average */
    double GetCorTimeAve() const {
        return ((left_.GetWalkCorrectedTime() +
                 right_.GetWalkCorrectedTime()) * 0.5);
    }

    /** \return the The walk corrected time difference */
    double GetCorTimeDiff() const {
        return (left_.GetWalkCorrectedTime() -
                right_.GetWalkCorrectedTime() +
                GetCalibration().GetLeftRightTimeOffset());
    }

    /** \return the average high resolution X position */
    double GetAverageZPos() const {
        return ( aright_.GetHighResZPos() + aleft_.GetHighResZPos() ) / 2.0;
    }

    /**  \return the average high resolution Y position */
    double GetAverageYPos() const {
        return ( aright_.GetHighResYPos() + aleft_.GetHighResYPos() ) / 2.0;
    }

    double GetFT() const {
        return ( GetLeftPos().GetFTQdc() + GetRightPos().GetFTQdc() );
    }
    double GetFB() const {
        return ( GetLeftPos().GetFBQdc() + GetRightPos().GetFBQdc() );
    }
    double GetBT() const {
        return ( GetLeftPos().GetBTQdc() + GetRightPos().GetBTQdc() );
    }
    double GetBB() const {
        return ( GetLeftPos().GetBBQdc() + GetRightPos().GetBBQdc() );
    }

    /** \return the left_ var */
    const HighResTimingData &GetLeftSide() const { return (left_); }

    /** \return the right_ var */
    const HighResTimingData &GetRightSide() const { return (right_); }

    const HighResPositionData &GetLeftPos() const { return (aleft_); }

    const HighResPositionData &GetRightPos() const { return (aright_); }

    /** \return the type of bar detector */
    std::string GetType() const { return (key_.second); }

    /** \return the time calibration var */
    const TimingCalibration GetCalibration() const {
        return (TimingCalibrator::get()->GetCalibration(key_));
    }

private:
    HighResTimingData right_; //!< The Right side of the detector
    HighResTimingData left_; //!< The Left side of the detector
    HighResPositionData aleft_;
    HighResPositionData aright_;
    TimingDefs::TimingIdentifier key_; //!< The key for the detector 
};

/** Defines a map to hold NEXT Detectors */
typedef std::map<TimingDefs::TimingIdentifier, NEXTDetector> NEXTMap;
#endif // __NEXTDETECTOR_HPP__
