/** \file BetaScintProcessor.hpp
 *
 * Processor for beta scintillator detectors
 */

#ifndef __BETASCINTPROCESSOR_HPP_
#define __BETASCINTPROCESSOR_HPP_

#include "EventProcessor.hpp"

namespace dammIds {
    namespace beta_scint {
        const int D_MULT_BETA = 0;
        const int D_ENERGY_BETA = 1;
        const int D_MULT_BETA_GATED = 2;
        const int D_ENERGY_BETA_GATED = 3;

        const int DD_ENERGY_BETA__TIME_TOTAL = 10;
        const int DD_ENERGY_BETA__TIME_NOG = 11;
        const int DD_ENERGY_BETA__TIME_G = 12;

        const int DD_ENERGY_BETA__TIME_TM_TOTAL = 20;
        const int DD_ENERGY_BETA__TIME_TM_NOG = 21;
        const int DD_ENERGY_BETA__TIME_TM_G = 22;
    }
} 

class BetaScintProcessor : public EventProcessor
{
public:
    BetaScintProcessor(double gammaBetaLimit);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
    static const double timeSpectraEnergyContraction = 10.0;
    static const double timeSpectraTimeResolution = 10e-3;

protected:
    EventData BestGammaForBeta(double bTime);
    /** Returns true if gamma-beta correlation time is within limits. */
    bool GoodGammaBeta(double gTime);
    double gammaBetaLimit_;
};

#endif // __BETASCINTPROCSSEOR_HPP_
