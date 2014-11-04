/** \file TemplateProcessor.hpp
 * \brief A Template class to be used to build others.
 * \author S. V. Paulauskas
 * \date October 26, 2014
 */
#ifndef __TEMPLATEPROCESSOR_HPP__
#define __TEMPLATEPROCESSOR_HPP__

#include "EventProcessor.hpp"

//! A generic processor to be used as a template for others
class TemplateProcessor : public EventProcessor {
public:
    /** Default Constructor */
    TemplateProcessor();
    /** Default Destructor */
    ~TemplateProcessor(){};
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
    virtual bool RetrieveData(RawEvent &event);
    /** Analyze the retrieved data
     * \param [in] rawev : the raw event with the data */
    virtual void AnalyzeData(RawEvent& rawev);
};
#endif // __TEMPLATEPROCESSOR_HPP__
