/** \file SingleBetaProcessor.hpp
 * \brief New Processor to handle a generic single ended "beta" detector
 * \author T. T. King
 * \date June 17, 2019
 */
#ifndef __SINGLEBETAPROCESSOR_HPP__
#define __SINGLEBETAPROCESSOR_HPP__

#include "EventProcessor.hpp"
#include "HighResTimingData.hpp"

class SingleBetaProcessor : public EventProcessor {
public:
    /** Default Constructor */
    SingleBetaProcessor();

    /** Default Destructor */
    ~SingleBetaProcessor() {};

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
    processor_struct::SINGLEBETA SBstruc; //!<Root Struct

};

#endif // __DOUBLEBETAPROCESSOR_HPP__