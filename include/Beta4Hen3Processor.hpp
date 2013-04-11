/** \file Beta4Hen3Processor.hpp
 *
 * Processor for beta scintillator detectors for 3Hen hybrid experiment
 */

#ifndef __BETA4HEN3PROCESSOR_HPP_
#define __BETA4HEN3PROCESSOR_HPP_

#include "BetaScintProcessor.hpp"

class Beta4Hen3Processor : public BetaScintProcessor
{
public:
    Beta4Hen3Processor(double gammaBetaLimit, double energyContracion);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

#endif // __BETA4HEN3PROCSSEOR_HPP_
