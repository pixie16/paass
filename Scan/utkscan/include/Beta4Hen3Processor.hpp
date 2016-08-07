/** \file Beta4Hen3Processor.hpp
 *
 * Processor for beta scintillator detectors for 3Hen hybrid experiment
 */

#ifndef __BETA4HEN3PROCESSOR_HPP_
#define __BETA4HEN3PROCESSOR_HPP_

#include "BetaScintProcessor.hpp"

///Class to handle Beta events for the 3Hen detector
class Beta4Hen3Processor : public BetaScintProcessor {
public:
    /** Constructor taking two arguments */
    Beta4Hen3Processor(double gammaBetaLimit, double energyContracion);
    /** Process the events */
    virtual bool Process(RawEvent &event);
    /** Declare the plots */
    virtual void DeclarePlots(void);
};

#endif // __BETA4HEN3PROCSSEOR_HPP_
