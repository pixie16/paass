/*! \file Ge4Hen3Processor.hpp
 *
 * Processor for germanium detectors for 3Hen experiment
 */

#ifndef __GE4HEN3PROCESSOR_HPP_
#define __GE4HEN3PROCESSOR_HPP_

#include <map>
#include <vector>

#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "GeProcessor.hpp"

class Ge4Hen3Processor : public GeProcessor
{
protected:

public:
    Ge4Hen3Processor(double gammaThreshold, double lowRatio,
                double highRatio, double subEventWindow,
                double gammaBetaLimit, double gammaGammaLimit,
                double earlyLowLimit, double earlyHighLimit);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

#endif // __GE4HEN3PROCESSOR_HPP_
