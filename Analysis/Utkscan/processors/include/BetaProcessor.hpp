/** \file BetaProcessor.hpp
 *
 * Processor for beta scintillator detectors
 */
#ifndef __BETAPROCESSOR_HPP_
#define __BETAPROCESSOR_HPP_

#include "EventProcessor.hpp"

/// Detector processor that handles scintillator events for beta detection (deprecated)
class BetaProcessor : public EventProcessor {
public:
    /*! Default Constructor */
    BetaProcessor();
    /*! Default Destructor */
    ~BetaProcessor() {};
    /*! \brief PreProcessing for the class
    *
    * \param [in] event : The RawEvent
    * \return [out] bool : Status of processing
    */
    virtual bool PreProcess(RawEvent &event);
    /*! \brief Main Processing for the class
    *
    * \param [in] event : The RawEvent
    * \return [out] bool : Status of processing
    */
    virtual bool Process(RawEvent &event);
    /*! Declare the Plots for the Processor */
    virtual void DeclarePlots(void);
};
#endif // __BETAPROCSSEOR_HPP_
