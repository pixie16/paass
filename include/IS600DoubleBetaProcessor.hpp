/** \file IS600DoubleBetaProcessor.hpp
 * \brief A Template class to be used to build others.
 * \author S. V. Paulauskas
 * \date October 26, 2014
 */
#ifndef __IS600DOUBLEBETAPROCESSOR_HPP__
#define __IS600DOUBLEBETAPROCESSOR_HPP__

#include "BarDetector.hpp"
#include "EventProcessor.hpp"
#include "HighResTimingData.hpp"
#include "DoubleBetaProcessor.hpp"

//! A generic processor to be used as a template for others
class IS600DoubleBetaProcessor : public DoubleBetaProcessor {
public:
    /** Default Constructor */
    IS600DoubleBetaProcessor();
    /** Default Destructor */
    ~IS600DoubleBetaProcessor(){};

    /** Declares the plots for the processor */
    virtual void DeclarePlots(void);
    /** Performs the preprocessing, which cannot depend on other processors
    * \param [in] event : the event to process
    * \return true if preprocessing was successful */
    virtual bool PreProcess(RawEvent &event);
    /** Performs the main processsing, which may depend on other processors
    * \param [in] event : the event to process
    * \return true if processing was successful */
    virtual bool Process(RawEvent &event);
private:
    BarMap betas_;//!< A map to hold all of the beta events.
};
#endif // __IS600DOUBLEBETAPROCESSOR_HPP__
