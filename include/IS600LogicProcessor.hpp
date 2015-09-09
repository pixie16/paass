/** \file IS600LogicProcessor.hpp
 * \brief class to handle mtc signals
 */
#ifndef __IS600LOGICPROCESSOR_HPP_
#define __IS600LOGICPROCESSOR_HPP_

#include "MtcProcessor.hpp"

//! Processor to handle signals from the Moving Tape Collector
class IS600LogicProcessor : public MtcProcessor{
 public:
    /** Enable/disable double stop / start flag
     * \param [in] double_stop : sets the double_stop flag
     * \param [in] double_start : set the double_start flag
     */
    IS600LogicProcessor(bool double_stop, bool double_start);

    /** Declare the plots for the processor */
    virtual void DeclarePlots(void);
    /** Preprocess the event
    * \param [in] event : the event to process
    * \return true if the preprocess was successful */
    virtual bool PreProcess(RawEvent &event);
    /** Process the event
    * \param [in] event : the event to process
    * \return true is the processing was successful */
    virtual bool Process(RawEvent &event);
 private:
    /** In some experiments the MTC stop signal was doubled
     * this flags enable removal of such an events */
    bool double_stop_;

    /** In some experiments the MTC start signal was doubled
     * this flags enable removal of such an events */
    bool double_start_;

    /** Upper limit in seconds for bad (double) start/stop event */
    static const double doubleTimeLimit_ = 10e-6;
};
#endif // __IS600LOGICPROCESSOR_HPP_
