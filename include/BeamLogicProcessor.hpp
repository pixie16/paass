/** \file BeamLogicProcessor.hpp
 *
 * Class to handle beam logic signals
 * To be used with TreeCorrelator
 * originally develeped for Dubna SHE 2014
 */

#ifndef __BEAMLOGICPROCESSOR_HPP_
#define __BEAMLOGICPROCESSOR_HPP_

#include "EventProcessor.hpp"

class BeamLogicProcessor : public EventProcessor {
public:
    BeamLogicProcessor();
    BeamLogicProcessor(int offset, int range);
    virtual void DeclarePlots(void);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);

private:
    /** Upper limit in seconds for bad (double) start/stop event */
    static const double doubleTimeLimit_ = 1.0e-6;
};

#endif // __BEAMLOGICPROCESSOR_HPP_
