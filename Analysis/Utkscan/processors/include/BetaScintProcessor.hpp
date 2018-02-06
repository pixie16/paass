/** \file BetaScintProcessor.hpp
 *
 * Processor for beta scintillator detectors
 * \author K. A. Miernik
 * \date 2012
 */
#ifndef __BETASCINTPROCESSOR_HPP_
#define __BETASCINTPROCESSOR_HPP_

#include "EventProcessor.hpp"

namespace dammIds {
    /*! Namespace containing plot numbers for beta_scint */
    namespace beta_scint {
        const int D_MULT_BETA = 0; //!< Multiplicity
        const int D_ENERGY_BETA = 1;//!< Energy
        const int D_MULT_BETA_THRES_GATED = 2;//!< Multi beta threshold gated
        const int D_ENERGY_BETA_THRES_GATED = 3;//!< Energy threshold gated

        const int D_MULT_BETA_GAMMA_GATED = 4;//!< Multi beta gamma gated
        const int D_ENERGY_BETA_GAMMA_GATED = 5;//!< Energy (gamma gated)
        const int DD_ENERGY_BETA__GAMMA = 6;//!< Beta Energy vs. Gamma Energy

        const int DD_ENERGY_BETA__TIME_TOTAL = 10;//!< Energy vs. Beta Time Total
        const int DD_ENERGY_BETA__TIME_NOG = 11;//!< Energy vs. Beta Time (w/o gamma)
        const int DD_ENERGY_BETA__TIME_G = 12;//!< Energy vs. Beta Time (w gamma)

        const int DD_ENERGY_BETA__TIME_TM_TOTAL = 20;//!< Energy vs. Beta Time (tape move)
        const int DD_ENERGY_BETA__TIME_TM_NOG = 21;//!< Energy vs. Beta Time (tape move - w/o gamma)
        const int DD_ENERGY_BETA__TIME_TM_G = 22;//!< Energy vs. Beta Time (tape move - w/ gamma)
    }
}

/// Detector processor that handles scintillator detectors for beta detection
class BetaScintProcessor : public EventProcessor {
public:
    /*! Default Constructor */
    BetaScintProcessor() {};
    /*! Default Destructor */
    ~BetaScintProcessor() {};
    /** Constructor taking limits on beta-gamma correlation and energy contraction
     * \param [in] gammaBetaLimit : the maximum time diff between beta and gamma events
     * \param [in] energyContraction : the number to contract the energy by */
    BetaScintProcessor(double gammaBetaLimit, double energyContraction);
    /*! \brief PreProcessing for the class
    * \param [in] event : The RawEvent
    * \return bool : Status of processing */
    virtual bool PreProcess(RawEvent &event);
    /*! \brief Main Processing for the class
    * \param [in] event : The RawEvent
    * \return bool : Status of processing */
    virtual bool Process(RawEvent &event);
    /*! Declare the Plots for the Processor */
    virtual void DeclarePlots(void);
protected:
    /*! Finds the most likely gamma associated with a given beta
    * \param [in] bTime : The time of arrival for the beta particle
    * \return EventData for the match */
    EventData BestGammaForBeta(double bTime);
    /** \return true if gamma-beta correlation time is within limits.
     * \param [in] gTime : the gamma time to check for coincidence  */
    bool GoodGammaBeta(double gTime);
    /** Gamma-beta coin. limit in seconds */
    double gammaBetaLimit_;
    /** Contraction of beta energy for 2d plots (time-energy and gamma-beta
     * energy */
    double energyContraction_;
};

#endif // __BETASCINTPROCSSEOR_HPP_
