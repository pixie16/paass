/** \file NeutronProcessor.hpp
 *
 * Processor for neutron scintillator detectors
 */

#ifndef __NEUTRONPROCESSOR_HPP_
#define __NEUTRONPROCESSOR_HPP_

#include "EventProcessor.hpp"

//! Processor for handling scintillator neutron detectors -  Deprecated
class NeutronProcessor : public EventProcessor {
public:
    /** Default Constructor */
    NeutronProcessor();

    /** Default Destructor */
    ~NeutronProcessor();

    /** Performs the preprocessing, which cannot depend on other processors
    * \param [in] event : the event to process
    * \return true if preprocessing was successful */
    virtual bool PreProcess(RawEvent &event);

    /** Process an event
    * \param [in] event : the event to process
    * \return true if the processing was successful */
    virtual bool Process(RawEvent &event);

    /** Declare plots for processor */
    virtual void DeclarePlots(void);
};

#endif // __NEUTRONPROCSSEOR_HPP_
