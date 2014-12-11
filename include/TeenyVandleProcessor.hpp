/** \file TeenyVandleProcessor.hpp
 * \brief A Template class to be used to build others.
 * \author S. V. Paulauskas
 * \date December 11, 2014
 */
#ifndef __TEENYVANDLEPROCESSOR_HPP__
#define __TEENYVANDLEPROCESSOR_HPP__

#include "EventProcessor.hpp"
#include "HighResTimingData.hpp"

//! A generic processor to be used as a template for others
class TeenyVandleProcessor : public EventProcessor {
public:
    /** Default Constructor */
    TeenyVandleProcessor();
    /** Default Destructor */
    ~TeenyVandleProcessor(){};
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
    /** Retrieves the data for the associated types
     * \param [in] event : the event to get data from
     * \return True if it could get data */
    //virtual bool RetrieveData(RawEvent &event);
    /** Analyze the retrieved data
     * \param [in] rawev : the raw event with the data */
    //virtual void AnalyzeData(RawEvent& rawev);

    TimingMap data_;
};
#endif // __TEENYVANDLEPROCESSOR_HPP__
