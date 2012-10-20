/** \file BetaScintProcessor.hpp
 *
 * Processor for beta scintillator detectors
 */

#ifndef __BETASCINTPROCESSOR_HPP_
#define __BETASCINTPROCESSOR_HPP_

#include "EventProcessor.hpp"

class BetaScintProcessor : public EventProcessor
{
public:
    BetaScintProcessor();
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

#endif // __BETASCINTPROCSSEOR_HPP_
