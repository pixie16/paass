/*! \file IS600GeProcessor.hpp
 *
 * Processor for germanium detectors for 3Hen experiment
 */

#ifndef __IS600GEPROCESSOR_HPP_
#define __IS600GEPROCESSOR_HPP_

#include <map>
#include <vector>

#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "GeProcessor.hpp"

class IS600GeProcessor : public GeProcessor
{
protected:

public:
    IS600GeProcessor(double gammaThreshold, double lowRatio,
                double highRatio, double subEventWindow,
                double gammaBetaLimit, double gammaGammaLimit,
                double cycle_gate1_min, double cycle_gate1_max,
                double cycle_gate2_min, double cycle_gate2_max);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

#endif // __IS600GEPROCESSOR_HPP_
