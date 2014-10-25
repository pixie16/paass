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

/*! TimingCalibrator */
class TimingCalibrator {
public:
    static TimingCalibrator* get();
private:
    TimingCalibrator() {ReadTimingCalXml();};

    void ReadTimingCalXml();

    TimingCalibrator (const TimingCalibrator&);
    TimingCalibrator& operator = (TimingCalibrator const&);
    static TimingCalibrator* instance;
};
#endif // __TIMINGCALIBRATOR_HPP__
