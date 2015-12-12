/** \file PulserProcessor.hpp
 * \brief Class for handling Pulser Signals
 * \author S. V. Paulauskas
 * \date 10 July 2009
 */

#ifndef __PULSERPROCESSOR_HPP_
#define __PULSERPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "HighResTimingData.hpp"

//! A class to handle time differences between two pulsers
class PulserProcessor : public EventProcessor {
 public:
    /** Default Constructor */
    PulserProcessor();
    /** Default Destructor */
    ~PulserProcessor(){};
    /** Declares the plots for the processor */
    virtual void DeclarePlots(void);
    /** Performs the main processsing, which may depend on other processors
    * \param [in] event : the event to process
    * \return true if processing was successful */
    virtual bool Process(RawEvent &event);
 protected:
    TimingMap pulserMap_; //!< map of the pulser events
 private:
    /** Retrieves the data for the associated types
     * \param [in] event : the event to get data from
     * \return True if it could get data */
    virtual bool RetrieveData(RawEvent &event);
    /** Analyze the retrieved data */
    virtual void AnalyzeData(void);
};
#endif // __PULSERPROCESSOR_HPP_
