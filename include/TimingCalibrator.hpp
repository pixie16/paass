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
    TimingCalibration(){};
    /** Default Destructor */
    ~TimingCalibration(){};

    /** \return The left-right time offset in ns */
    double GetLeftRightTimeOffset(void)const {return(lrtOffset_);};
    /** \return the distance between source and the hit location in the bar in cm */
    double GetR0(void) const {return(r0_);};
    /** \return time offset w.r.t. start number 0 in ns */
    double GetTofOffset0(void) const {return(tofOffset0_);};
    /** \return time offset w.r.t. start number 1 in ns */
    double GetTofOffset1(void) const {return(tofOffset1_);};
    /** \return offset between the center of the bar and the source in cm */
    double GetXOffset(void) const {return(xOffset_);};
    /** \return the perpendicular distance between the bar and source in cm */
    double GetZ0(void) const {return(z0_);};
    /** \return additional corrections to z0 in cm */
    double GetZOffset(void) const {return(zOffset_);};

    /** Sets the left-right time offset
    * \param [in] a : the offset in ns */
    void SetLeftRightTimeOffset(const double &a) {lrtOffset_ = a;};
    /** Sets the distance between source and the hit location in the bar
    * \param [in] a : the distance in cm */
    void SetR0(const double &a) {r0_ = a;};
    /** Sets the time offset w.r.t. start number 0
    * \param [in] a : the offset in ns */
    void SetTofOffset0(const double &a) {tofOffset0_ = a;};
    /** Sets the time offset w.r.t. start number 1
    * \param [in] a : the offset in ns */
    void SetTofOffset1(const double &a) {tofOffset1_ = a;};
    /** Sets the offset between the center of the bar and the source
    * \param [in] a : the offset in cm */
    void SetXOffset(const double &a) {xOffset_ = a;};
    /** Sets the perpendicular distance between the bar and source
    * \param [in] a : the distance in cm */
    void SetZ0(const double &a) {z0_ = a;};
    /** Sets the additional corrections to z0
    * \param [in] a : the offset in cm */
    void SetZOffset(const double &a) {zOffset_ = a;};
private:
    double lrtOffset_;//!< left-right time offset
    double r0_;//!< distance between source and the hit location in the bar
    double tofOffset0_;//!< time offset w.r.t. start number 0
    double tofOffset1_;//!< time offset w.r.t. start number 1
    double xOffset_;//!< offset between the center of the bar and the source
    double z0_;//!< perpendicular distance between the bar and source
    double zOffset_;//!< additional corrections to z0
};

/*! \brief Class to handle time calibrations for bar type detectors - Singleton
*
* It is important to note that "left" generally refers to the upstream side of
* the bar detector */
class TimingCalibrator {
public:
    /** \return Instance of the TimingCalibrator class */
    static TimingCalibrator* get();

    /** \return The calibration for the requested bar
     * \param [in] id : the id of the bar that you want the calibration for */
    TimingCalibration GetCalibration(const TimingDefs::TimingIdentifier &id);
private:
    TimingCalibrator() {ReadTimingCalXml();}; //!<Default constructor
    TimingCalibrator (const TimingCalibrator&);//!< Overload of the constructor
    TimingCalibrator& operator = (TimingCalibrator const&);//!< the copy constructor
    static TimingCalibrator* instance; //!< static instance of the class

    void ReadTimingCalXml(); //!< Reads in the calibrations in the XML config

    Messenger m_; //!< Instance of the Messenger class to output information
    std::map <TimingDefs::TimingIdentifier, TimingCalibration> calibrations_; //!< map to hold the calibrations
};
#endif // __TIMINGCALIBRATOR_HPP__
