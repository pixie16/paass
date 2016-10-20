/** \file E14060Processor.hpp
 * \brief Experiment specific processor to handle the e14060 experiment at
 * the NSCL.
 *\author S. V. Paulauskas
 *\date September 15, 2016
 */
#ifndef __E14060PROCESSOR_HPP_
#define __E14060PROCESSOR_HPP_

#include <fstream>

#include "EventProcessor.hpp"

#ifdef useroot

#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH1D.h>

#endif

/// Class to analyze data from e14060
class E14060Processor : public EventProcessor {
public:
    ///Constructor that will accept a user defined range of Clover energys to
    /// gate the PID.
    E14060Processor(std::pair<double, double> &energyRange);

    /** Default Destructor */
    ~E14060Processor() {};

    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    /** Process the event
    * \param [in] event : the event to process
    * \return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);

private:
    std::pair<double, double> energyRange_; ///!< Ge range ge for cuts on PID

    ///@brief A method that will plot the PID spectra given the inputs.
    ///@param[in] tacs : Vector containing TAC events to plot
    ///@param[in] pins : vector containing PIN events to plot
    ///@param[in] hasGe : True if we had a Ge event in the desired range
    ///@param[in] hasImplant : True if we had the PSPMT fire in the event.
    void PlotPid(const std::vector<ChanEvent *> &tacs,
                 const std::vector<ChanEvent *> &pins,
                 const bool &hasGe, const bool &hasPspmt);

    ///@brief A method that will set the types associated with this
    /// processing class
    void SetAssociatedTypes();
};

#endif
