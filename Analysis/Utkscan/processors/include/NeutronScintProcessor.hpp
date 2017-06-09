/** \file NeutronScintProcessor.hpp
 *
 * Processor for neutron scintillator detectors
 * @authors K. Miernik, S. V. Paulauskas
 */

#ifndef __NEUTRONSCINTPROCESSOR_HPP_
#define __NEUTRONSCINTPROCESSOR_HPP_

#include "EventProcessor.hpp"

//! Class to handle Neutron Scintillators (that are not VANDLE)
class NeutronScintProcessor : public EventProcessor {
public:
    /** Default Constructor */
    NeutronScintProcessor();

    /** Preprocess the event
     * \param [in] event : the event to preprocess
     * \return true if successful */
    virtual bool PreProcess(RawEvent &event);

    /** Process the event
     * \param [in] event : the event to process
     * \return true if successful */
    virtual bool Process(RawEvent &event);

    /** Declare the plots for the processor */
    virtual void DeclarePlots(void);
};

#endif // __NEUTRONSCINTPROCSSEOR_HPP_
