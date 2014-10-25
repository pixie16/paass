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

class TimingCalibration {
public:
    TimingCalibration(){};
    ~TimingCalibration(){};

    double GetLeftRightTimeOffset(void)const {return(lrtOffset_);};
    double GetR0(void) const {return(r0_);};
    double GetTofOffset0(void) const {return(tofOffset0_);};
    double GetTofOffset1(void) const {return(tofOffset1_);};
    double GetXOffset(void) const {return(xOffset_);};
    double GetZ0(void) const {return(z0_);};
    double GetZOffset(void) const {return(zOffset_);};

    void SetLeftRightTimeOffset(const double &a) {lrtOffset_ = a;};
    void SetR0(const double &a) {r0_ = a;};
    void SetTofOffset0(const double &a) {tofOffset0_ = a;};
    void SetTofOffset1(const double &a) {tofOffset1_ = a;};
    void SetXOffset(const double &a) {xOffset_ = a;};
    void SetZ0(const double &a) {z0_ = a;};
    void SetZOffset(const double &a) {zOffset_ = a;};
private:
    double lrtOffset_, r0_, tofOffset0_, tofOffset1_, xOffset_, z0_, zOffset_;
};

/*! TimingCalibrator */
class TimingCalibrator {
public:
    static TimingCalibrator* get();

    TimingCalibration GetCalibration(const Vandle::BarIdentifier &id);
private:
    TimingCalibrator() {ReadTimingCalXml();};
    TimingCalibrator (const TimingCalibrator&);
    TimingCalibrator& operator = (TimingCalibrator const&);
    static TimingCalibrator* instance;

    void ReadTimingCalXml();

    Messenger m_;
    std::map <Vandle::BarIdentifier, TimingCalibration> calibrations_;
};
#endif // __TIMINGCALIBRATOR_HPP__
