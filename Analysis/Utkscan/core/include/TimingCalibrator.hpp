/*! \file TimingCalibrator.hpp
 *  \brief A class to handle the timing calibration parameters
 *  \author S. V. Paulauskas
 *  \date October 23, 2014
*/
#ifndef __TIMINGCALIBRATOR_HPP__
#define __TIMINGCALIBRATOR_HPP__

#include <vector>
#include <map>
#include <string>

#include "Globals.hpp"
#include "Messenger.hpp"

//! A class to hold the timing calibration for a detector
class TimingCalibration {
public:
    /** Default Constructor */
    TimingCalibration() { SetDefaults(); };

    /** Default Destructor */
    ~TimingCalibration() {};

    /** \return The left-right time offset in ns */
    double GetLeftRightTimeOffset(void) const { return (lrtOffset_); };

    /** \return time offset w.r.t. start at location a
     * \param [in] a : location of the start to get the offset for*/
    double GetTofOffset(const unsigned int &a) const {
        if (tofOffsets_.find(a) == tofOffsets_.end())
            return (0.0);
        return ((*tofOffsets_.find(a)).second);
    };

    /** \return offset between the center of the bar and the source in cm */
    double GetXOffset(void) const { return (xOffset_); };

    /** \return the perpendicular distance between the bar and source in cm */
    double GetZ0(void) const { return (z0_); };

    /** \return additional corrections to z0 in cm (UNUSED)*/
    double GetZOffset(void) const { return (zOffset_); };

    /** \return the number of ToF Offsets that were read in from the calibration */
    unsigned int GetNumTofOffsets(void) const { return (tofOffsets_.size()); };

    /** Sets the left-right time offset
    * \param [in] a : the offset in ns */
    void SetLeftRightTimeOffset(const double &a) { lrtOffset_ = a; };

    /** Sets the time offset w.r.t. to start location
    * \param [in] a : the start location
    * \param [in] b : the offset in ns */
    void SetTofOffset(const unsigned int &a, const double &b) {
        tofOffsets_.insert(std::make_pair(a, b));
    };

    /** Sets the offset between the center of the bar and the source
    * \param [in] a : the offset in cm */
    void SetXOffset(const double &a) { xOffset_ = a; };

    /** Sets the perpendicular distance between the bar and source
    * \param [in] a : the distance in cm */
    void SetZ0(const double &a) { z0_ = a; };

    /** Sets the additional corrections to z0
    * \param [in] a : the offset in cm */
    void SetZOffset(const double &a) { zOffset_ = a; };
private:
    double lrtOffset_;//!< left-right time offset
    double xOffset_;//!< offset between the center of the bar and the source
    double z0_;//!< perpendicular distance between the bar and source
    double zOffset_;//!< additional corrections to z0

    std::map<unsigned int, double> tofOffsets_; //!< A map holding all of the ToF offsets w.r.t. starts

    /** Initializer for a default timing calibration */
    void SetDefaults(void) {
        lrtOffset_ = xOffset_ = z0_ = zOffset_ = 0.0;
    };
};

/*! \brief Class to handle time calibrations for bar type detectors - Singleton
*
* It is important to note that "left" generally refers to the upstream side of
* the bar detector */
class TimingCalibrator {
public:
    /** \return Instance of the TimingCalibrator class */
    static TimingCalibrator *get();

    /** \return The calibration for the requested bar
     * \param [in] id : the id of the bar that you want the calibration for */
    TimingCalibration GetCalibration(const TimingDefs::TimingIdentifier &id);

private:
    TimingCalibrator() { ReadTimingCalXml(); }; //!<Default constructor
    TimingCalibrator(const TimingCalibrator &);//!< Overload of the constructor
    TimingCalibrator &
    operator=(TimingCalibrator const &);//!< the copy constructor
    static TimingCalibrator *instance; //!< static instance of the class

    void ReadTimingCalXml(); //!< Reads in the calibrations in the XML config

    Messenger m_; //!< Instance of the Messenger class to output information
    std::map <TimingDefs::TimingIdentifier, TimingCalibration> calibrations_; //!< map to hold the calibrations
    TimingCalibration default_; //!< A default (all zeroes) calibration to return if none present
    bool isVerbose_; //!< The setting of the verbosity of the Timing Calibrations
};

#endif // __TIMINGCALIBRATOR_HPP__
