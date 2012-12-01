/** \file BetaProcessor.hpp
 *
 * Processor for beta scintillator detectors
 */

#ifndef __BETAPROCESSOR_HPP_
#define __BETAPROCESSOR_HPP_

#include "EventProcessor.hpp"

class BetaProcessor : public EventProcessor
{
public:
    BetaProcessor();
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

#endif // __BETAPROCSSEOR_HPP_
