/** \file BeamLogicProcessor.hpp
 *
 * Class to handle beam logic signals
 * To be used with TreeCorrelator
 * originally develeped for Dubna SHE 2014
 */
#ifndef __BEAMLOGICPROCESSOR_HPP_
#define __BEAMLOGICPROCESSOR_HPP_

#include "EventProcessor.hpp"

/// Processor to handle Beam Logic for Dubna SHE 2014
class BeamLogicProcessor : public EventProcessor {
public:
    /*! Default Constructor */
    BeamLogicProcessor();
    /*! Default Destructor */
    BeamLogicProcessor(int offset, int range);
    /*! Declare the Plots for the Processor */
    virtual void DeclarePlots(void);
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

private:
    /*! Upper limit in seconds for bad (double) start/stop event */
    static const double doubleTimeLimit_ = 1.0e-6;
};

#endif // __BEAMLOGICPROCESSOR_HPP_
