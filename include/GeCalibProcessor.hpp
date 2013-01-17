/*! \file GeCalibProcessor.hpp
 *
 * Processor for germanium detectors - calibration 
 */

#ifndef __GECALIBPROCESSOR_HPP_
#define __GECALIBPROCESSOR_HPP_

#include <map>
#include <vector>

#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "GeProcessor.hpp"

class GeCalibProcessor : public GeProcessor
{
protected:

public:
    GeCalibProcessor();
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

#endif // __GECALIBPROCESSOR_HPP_
